/*
 * Copyright 2017-2020 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils/json_reader.h"
#include "utils/yaml_reader.h"
#include "utils/file.h"
#include "utils/string.h"
#include "openrasp.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/no_params_object.h"
#include "utils/signal_interceptor.h"

extern "C"
{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}
#include "openrasp_log.h"
#include "openrasp_v8.h"
#include "openrasp_hook.h"
#include "openrasp_inject.h"
#include "openrasp_security_policy.h"
#include "openrasp_output_detect.h"
#include "openrasp_check_type.h"
#ifdef HAVE_FSWATCH
#include "openrasp_fswatch.h"
#endif
#include <new>
#include "agent/shared_config_manager.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

using openrasp::ConfigHolder;

ZEND_DECLARE_MODULE_GLOBALS(openrasp);

const char *OpenRASPInfo::PHP_OPENRASP_VERSION = "1.3.5";
bool is_initialized = false;
bool remote_active = false;
std::string openrasp_status = "Protected";
static std::string get_complete_config_content(ConfigHolder::FromType type);
static void hook_without_params(OpenRASPCheckType check_type TSRMLS_DC);

PHP_INI_BEGIN()
PHP_INI_ENTRY1("openrasp.root_dir", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.root_dir)
#ifdef HAVE_GETTEXT
PHP_INI_ENTRY1("openrasp.locale", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.locale)
#endif
PHP_INI_ENTRY1("openrasp.backend_url", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.backend_url)
PHP_INI_ENTRY1("openrasp.app_id", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.app_id)
PHP_INI_ENTRY1("openrasp.app_secret", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.app_secret)
PHP_INI_ENTRY1("openrasp.rasp_id", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.rasp_id)
PHP_INI_ENTRY1("openrasp.remote_management_enable", "0", PHP_INI_SYSTEM, OnUpdateOpenraspBool, &openrasp_ini.remote_management_enable)
PHP_INI_ENTRY1("openrasp.heartbeat_interval", "180", PHP_INI_SYSTEM, OnUpdateOpenraspHeartbeatInterval, &openrasp_ini.heartbeat_interval)
PHP_INI_ENTRY1("openrasp.ssl_verifypeer", "0", PHP_INI_SYSTEM, OnUpdateOpenraspBool, &openrasp_ini.ssl_verifypeer)
PHP_INI_ENTRY1("openrasp.iast_enable", "0", PHP_INI_SYSTEM, OnUpdateOpenraspBool, &openrasp_ini.iast_enable)
PHP_INI_END()

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
static PHP_FUNCTION(openrasp_ob_handler);
ZEND_BEGIN_ARG_INFO_EX(arginfo_openrasp_ob_handler, 0, 0, 1)
ZEND_ARG_INFO(0, input)
ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

static const zend_function_entry openrasp_functions[] = {
    PHP_FE(openrasp_ob_handler, arginfo_openrasp_ob_handler)
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 3 && PHP_RELEASE_VERSION < 7
        {NULL, NULL, NULL}
#else
        PHP_FE_END
#endif
};

static PHP_FUNCTION(openrasp_ob_handler)
{
    openrasp_detect_output(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
#endif

PHP_GINIT_FUNCTION(openrasp)
{
#ifdef ZTS
    new (openrasp_globals) _zend_openrasp_globals;
    openrasp::YamlReader yaml_reader(get_complete_config_content(ConfigHolder::FromType::kYaml));
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (!openrasp::oam)
    {
        (openrasp_globals->config).update(&yaml_reader);
    }
#else
    (openrasp_globals->config).update(&yaml_reader);
#endif
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp)
{
#ifdef ZTS
    openrasp_globals->~_zend_openrasp_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp, PHP_GINIT(openrasp), PHP_GSHUTDOWN(openrasp));
    REGISTER_INI_ENTRIES();
    if (!current_sapi_supported())
    {
        openrasp_status = "Unprotected (unsupported SAPI)";
        return SUCCESS;
    }
    if (!make_openrasp_root_dir(openrasp_ini.root_dir TSRMLS_CC))
    {
        openrasp_status = "Unprotected ('openrasp.root_dir' initialization failed)";
        return SUCCESS;
    }
    std::string locale_path(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + "locale" + DEFAULT_SLASH);
    openrasp_set_locale(openrasp_ini.locale, locale_path.c_str());
    if (!openrasp_ini.verify_rasp_id())
    {
        openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("openrasp.rasp_id can only contain alphanumeric characters and is between 16 and 512 in length."));
        openrasp_status = "openrasp.rasp_id can only contain alphanumeric characters and is between 16 and 512 in length.";
        return SUCCESS;
    }
    openrasp::scm.reset(new openrasp::SharedConfigManager());
    if (!openrasp::scm->startup())
    {
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("Fail to startup SharedConfigManager."));
        openrasp_status = "Unprotected (shared memory application failed)";
        return SUCCESS;
    }

#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (need_alloc_shm_current_sapi() && openrasp_ini.remote_management_enable)
    {
        openrasp::oam.reset(new openrasp::OpenraspAgentManager());
        std::string error_msg;
        if (!openrasp_ini.verify_remote_management_ini(error_msg))
        {
            openrasp_status = error_msg;
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, error_msg.c_str());
            return SUCCESS;
        }
        remote_active = true;
    }
#endif

    if (PHP_MINIT(openrasp_log)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE)
    {
        openrasp_status = "Unprotected (log module initialization failed)";
        return SUCCESS;
    }

    if (!remote_active)
    {
        //load yaml config
        openrasp::YamlReader yaml_reader(get_complete_config_content(ConfigHolder::FromType::kYaml));
        yaml_reader.set_exception_report(true);
        if (yaml_reader.has_error())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("Fail to parse config, cuz of %s."),
                           yaml_reader.get_error_msg().c_str());
        }
        else
        {
            std::string unknown_yaml_keys = yaml_reader.detect_unknown_config_key();
            if (!unknown_yaml_keys.empty())
            {
                openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("Unknown config key [%s] found in yml configuration."),
                               unknown_yaml_keys.c_str());
            }
            openrasp::scm->set_debug_level(&yaml_reader);
            openrasp::scm->build_check_type_white_array(&yaml_reader);
            openrasp::scm->build_weak_password_array(&yaml_reader);
            OPENRASP_G(config).update(&yaml_reader);
        }
    }

    if (PHP_MINIT(openrasp_v8)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE)
    {
        openrasp_status = "Unprotected (v8 module initialization failed)";
        return SUCCESS;
    }
    int result;
    result = PHP_MINIT(openrasp_hook)(INIT_FUNC_ARGS_PASSTHRU);
    result = PHP_MINIT(openrasp_inject)(INIT_FUNC_ARGS_PASSTHRU);

#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (remote_active && openrasp::oam)
    {
        if (sapi_module.name && strcmp(sapi_module.name, "cgi-fcgi") == 0)
        {
            signal(SIGCHLD, SIG_IGN);
        }
        openrasp::oam->startup();
    }
#endif

#ifdef HAVE_FSWATCH
    if (!remote_active)
    {
        result = PHP_MINIT(openrasp_fswatch)(INIT_FUNC_ARGS_PASSTHRU);
    }
#endif
    result = PHP_MINIT(openrasp_security_policy)(INIT_FUNC_ARGS_PASSTHRU);
    result = PHP_MINIT(openrasp_output_detect)(INIT_FUNC_ARGS_PASSTHRU);
    is_initialized = true;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        sigset_t x;
        sigemptyset(&x);
        sigaddset(&x, SIGUSR1);
        sigprocmask(SIG_BLOCK, &x, nullptr);
        int result;
        if (!remote_active)
        {
#ifdef HAVE_FSWATCH
            result = PHP_MSHUTDOWN(openrasp_fswatch)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
#endif
        }
        result = PHP_MSHUTDOWN(openrasp_inject)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_hook)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_v8)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_log)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
        if (remote_active && openrasp::oam)
        {
            openrasp::oam->shutdown();
        }
        openrasp::oam.reset();
#endif
        openrasp::scm->shutdown();
        openrasp::scm.reset();
        remote_active = false;
        is_initialized = false;
        sigprocmask(SIG_UNBLOCK, &x, nullptr);
    }
    UNREGISTER_INI_ENTRIES();
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp, PHP_GSHUTDOWN(openrasp));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        zval *http_server = fetch_http_globals(TRACK_VARS_SERVER TSRMLS_CC);
        OPENRASP_G(request).update_id();
        if (nullptr != http_server && Z_TYPE_P(http_server) == IS_ARRAY)
        {
            OPENRASP_G(request).url.set_request_scheme(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "REQUEST_SCHEME"));
            OPENRASP_G(request).url.set_http_host(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "HTTP_HOST"));
            OPENRASP_G(request).url.set_server_name(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "SERVER_NAME"));
            OPENRASP_G(request).url.set_server_addr(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "SERVER_ADDR"));
            OPENRASP_G(request).url.set_request_uri(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "REQUEST_URI"));
            OPENRASP_G(request).url.set_query_string(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "QUERY_STRING"));
            OPENRASP_G(request).url.set_port(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "SERVER_PORT"));

            OPENRASP_G(request).set_method(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "REQUEST_METHOD"));
            OPENRASP_G(request).set_remote_addr(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "REMOTE_ADDR"));
            OPENRASP_G(request).set_document_root(fetch_outmost_string_from_ht(Z_ARRVAL_P(http_server), "DOCUMENT_ROOT"));

            std::map<std::string, std::string> header;
            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(http_server));
                 zend_hash_has_more_elements(Z_ARRVAL_P(http_server)) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(http_server)))
            {
                char *key = nullptr;
                ulong idx;
                int type = 0;
                zval **value;
                std::string header_key;
                type = zend_hash_get_current_key(Z_ARRVAL_P(http_server), &key, &idx, 0);
                if (type == HASH_KEY_IS_STRING &&
                    !(header_key = convert_to_header_key(key, strlen(key))).empty() &&
                    zend_hash_get_current_data(Z_ARRVAL_P(http_server), (void **)&value) == SUCCESS &&
                    Z_TYPE_PP(value) == IS_STRING)
                {
                    header[header_key] = std::string(Z_STRVAL_PP(value), Z_STRLEN_PP(value));
                }
            }
            OPENRASP_G(request).set_header(header);
        }
        int result;
        long config_last_update = openrasp::scm->get_config_last_update();
        if (config_last_update && config_last_update > OPENRASP_G(config).GetLatestUpdateTime())
        {
            openrasp::JsonReader json_reader(get_complete_config_content(ConfigHolder::FromType::kJson));
            if (OPENRASP_G(config).update(&json_reader))
            {
                OPENRASP_G(config).SetLatestUpdateTime(config_last_update);
            }
        }
        OPENRASP_G(request).set_body_length(OPENRASP_CONFIG(body.maxbytes));
        // openrasp_inject must be called before openrasp_log cuz of request_id
        result = PHP_RINIT(openrasp_inject)(INIT_FUNC_ARGS_PASSTHRU);
        result = PHP_RINIT(openrasp_log)(INIT_FUNC_ARGS_PASSTHRU);
        openrasp::general_signal_hook();
        result = PHP_RINIT(openrasp_hook)(INIT_FUNC_ARGS_PASSTHRU);
        result = PHP_RINIT(openrasp_v8)(INIT_FUNC_ARGS_PASSTHRU);
        result = PHP_RINIT(openrasp_output_detect)(INIT_FUNC_ARGS_PASSTHRU);
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
        if (remote_active && openrasp::oam)
        {
            zval *http_global_server = fetch_http_globals(TRACK_VARS_SERVER TSRMLS_CC);
            if (http_global_server)
            {
                const std::string webroot = fetch_outmost_string_from_ht(Z_ARRVAL_P(http_global_server), "DOCUMENT_ROOT");
                if (!webroot.empty() &&
                    openrasp::oam->path_writable() &&
                    !openrasp::oam->path_exist(zend_inline_hash_func(webroot.c_str(), webroot.length())))
                {
                    openrasp::oam->write_webroot_path(webroot.c_str());
                }
            }
        }
#endif
        hook_without_params(REQUEST TSRMLS_CC);
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        int result;
        hook_without_params(REQUEST_END TSRMLS_CC);
        result = PHP_RSHUTDOWN(openrasp_hook)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_RSHUTDOWN(openrasp_log)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_RSHUTDOWN(openrasp_inject)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        OPENRASP_G(request).clear();
    }
    return SUCCESS;
}

PHP_MINFO_FUNCTION(openrasp)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Status", openrasp_status.c_str());
    php_info_print_table_row(2, "Version", OpenRASPInfo::PHP_OPENRASP_VERSION);
#ifdef OPENRASP_BUILD_TIME
    php_info_print_table_row(2, "Build Time", OPENRASP_BUILD_TIME);
#endif
#ifdef OPENRASP_COMMIT_ID
    php_info_print_table_row(2, "Commit Id", OPENRASP_COMMIT_ID);
#else
    php_info_print_table_row(2, "Commit Id", "");
#endif
    php_info_print_table_row(2, "V8 Version", ZEND_TOSTR(V8_MAJOR_VERSION) "." ZEND_TOSTR(V8_MINOR_VERSION));
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (remote_active && openrasp::oam)
    {
        const char *plugin_version = openrasp::oam->get_plugin_version();
        php_info_print_table_row(2, "Plugin Version", plugin_version ? plugin_version : "");
    }
#endif
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}

/**  module depends
 */
#if ZEND_MODULE_API_NO >= 20050922
zend_module_dep openrasp_deps[] = {
    ZEND_MOD_REQUIRED("standard")
        ZEND_MOD_REQUIRED("json")
            ZEND_MOD_CONFLICTS("xdebug")
                ZEND_MOD_END};
#endif

zend_module_entry openrasp_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
    STANDARD_MODULE_HEADER_EX,
    NULL,
    openrasp_deps,
#else
    STANDARD_MODULE_HEADER,
#endif
    "openrasp",
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
    openrasp_functions,
#else
    NULL,
#endif
    PHP_MINIT(openrasp),
    PHP_MSHUTDOWN(openrasp),
    PHP_RINIT(openrasp),
    PHP_RSHUTDOWN(openrasp),
    PHP_MINFO(openrasp),
    OpenRASPInfo::PHP_OPENRASP_VERSION,
    STANDARD_MODULE_PROPERTIES};

#ifdef COMPILE_DL_OPENRASP
ZEND_GET_MODULE(openrasp)
#endif

static std::string get_complete_config_content(ConfigHolder::FromType type)
{
    std::string conf_content;
    if (openrasp_ini.root_dir)
    {
        std::string abs_path = std::string(openrasp_ini.root_dir) +
                               DEFAULT_SLASH +
                               "conf" +
                               DEFAULT_SLASH +
                               (ConfigHolder::FromType::kJson == type ? "cloud-config.json" : "openrasp.yml");
        openrasp::read_entire_content(abs_path.c_str(), conf_content);
    }
    return conf_content;
}

static void hook_without_params(OpenRASPCheckType check_type TSRMLS_DC)
{
    bool type_ignored = openrasp_check_type_ignored(check_type TSRMLS_CC);
    if (type_ignored)
    {
        return;
    }
    openrasp::data::NoParamsObject no_params_obj(check_type);
    openrasp::checker::V8Detector v8_detector(no_params_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}