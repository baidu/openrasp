/*
 * Copyright 2017-2018 Baidu Inc.
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

#include "openrasp.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"

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
#include "openrasp_fswatch.h"
#include <new>
#include "agent/openrasp_agent_manager.h"

ZEND_DECLARE_MODULE_GLOBALS(openrasp);

bool is_initialized = false;
static bool make_openrasp_root_dir(TSRMLS_D);

PHP_INI_BEGIN()
PHP_INI_ENTRY1("openrasp.root_dir", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.root_dir)
#ifdef HAVE_GETTEXT
PHP_INI_ENTRY1("openrasp.locale", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.locale)
#endif
PHP_INI_ENTRY1("openrasp.block_url", "https://rasp.baidu.com/blocked/", PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.block_url)
PHP_INI_ENTRY1("openrasp.slowquery_min_rows", "500", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.slowquery_min_rows)
PHP_INI_ENTRY1("openrasp.enforce_policy", "0", PHP_INI_SYSTEM, OnUpdateOpenraspBool, &openrasp_ini.enforce_policy)
PHP_INI_ENTRY1("openrasp.hooks_ignore", "sqlSlowQuery", PHP_INI_SYSTEM, OnUpdateOpenraspSet, &openrasp_ini.hooks_ignore)
PHP_INI_ENTRY1("openrasp.callable_blacklists", "system,exec,passthru,proc_open,shell_exec,popen,pcntl_exec,assert", PHP_INI_SYSTEM, OnUpdateOpenraspSet, &openrasp_ini.callable_blacklists)
PHP_INI_ENTRY1("openrasp.inject_urlprefix", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.inject_html_urlprefix)
PHP_INI_ENTRY1("openrasp.log_maxburst", "100", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.log_maxburst)
PHP_INI_ENTRY1("openrasp.syslog_server_address", nullptr, PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.syslog_server_address)
PHP_INI_ENTRY1("openrasp.syslog_facility", "1", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.syslog_facility)
PHP_INI_ENTRY1("openrasp.syslog_alarm_enable", "0", PHP_INI_SYSTEM, OnUpdateOpenraspBool, &openrasp_ini.syslog_alarm_enable)
PHP_INI_ENTRY1("openrasp.syslog_connection_timeout", "50", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.syslog_connection_timeout)
PHP_INI_ENTRY1("openrasp.syslog_read_timeout", "10", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.syslog_read_timeout)
PHP_INI_ENTRY1("openrasp.syslog_connection_retry_interval", "300", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.syslog_connection_retry_interval)
PHP_INI_ENTRY1("openrasp.timeout_ms", "100", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.timeout_ms)
PHP_INI_ENTRY1("openrasp.block_status_code", "302", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.block_status_code)
PHP_INI_ENTRY1("openrasp.plugin_maxstack", "100", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.plugin_maxstack)
PHP_INI_ENTRY1("openrasp.log_maxstack", "10", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.log_maxstack)
PHP_INI_ENTRY1("openrasp.backend", "http://scloud.baidu.com:8080", PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.backend)
PHP_INI_ENTRY1("openrasp.plugin_update_interval", "60", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.plugin_update_interval)
PHP_INI_ENTRY1("openrasp.log_push_interval", "10", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.log_push_interval)
PHP_INI_ENTRY1("openrasp.clientip_header", "clientip", PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.clientip_header)
PHP_INI_ENTRY1("openrasp.authentication_id", "4e56cd55aa559823e5dc256e486b2b50", PHP_INI_SYSTEM, OnUpdateOpenraspCString, &openrasp_ini.authentication_id)
PHP_INI_ENTRY1("openrasp.log_max_backup", "30", PHP_INI_SYSTEM, OnUpdateOpenraspIntGEZero, &openrasp_ini.log_max_backup)
PHP_INI_END()

PHP_GINIT_FUNCTION(openrasp)
{
#ifdef ZTS
    new (openrasp_globals) _zend_openrasp_globals;
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
    if (!make_openrasp_root_dir(TSRMLS_C))
    {
        openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.root_dir is not configured correctly in php.ini (not empty, not root path, not relative path and must be writable), continuing without security protection"));
        return SUCCESS;
    }
    if (PHP_MINIT(openrasp_log)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE)
    {
        return SUCCESS;
    }
    if (PHP_MINIT(openrasp_v8)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE)
    {
        return SUCCESS;
    }
    int result;
    result = PHP_MINIT(openrasp_hook)(INIT_FUNC_ARGS_PASSTHRU);
    result = PHP_MINIT(openrasp_inject)(INIT_FUNC_ARGS_PASSTHRU);
    result = PHP_MINIT(openrasp_security_policy)(INIT_FUNC_ARGS_PASSTHRU);
    openrasp::oam.startup();
    // result = PHP_MINIT(openrasp_fswatch)(INIT_FUNC_ARGS_PASSTHRU);
    is_initialized = true;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        int result;
        // result = PHP_MSHUTDOWN(openrasp_fswatch)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_inject)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_hook)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_v8)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_MSHUTDOWN(openrasp_log)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
    }
    openrasp::oam.shutdown();
    UNREGISTER_INI_ENTRIES();
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp, PHP_GSHUTDOWN(openrasp));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        int result;
        // openrasp_inject must be called before openrasp_log cuz of request_id
        result = PHP_RINIT(openrasp_inject)(INIT_FUNC_ARGS_PASSTHRU);
        result = PHP_RINIT(openrasp_log)(INIT_FUNC_ARGS_PASSTHRU);
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp)
{
    if (is_initialized)
    {
        int result;
        result = PHP_RSHUTDOWN(openrasp_log)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        result = PHP_RSHUTDOWN(openrasp_inject)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
    }
    return SUCCESS;
}

PHP_MINFO_FUNCTION(openrasp)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Status", is_initialized ? "Protected" : "Unprotected, Initialization Failed");
#ifdef OPENRASP_COMMIT_ID
    php_info_print_table_row(2, "Version", OPENRASP_COMMIT_ID);
#else
    php_info_print_table_row(2, "Version", "");
#endif
    php_info_print_table_row(2, "V8 Version", ZEND_TOSTR(V8_MAJOR_VERSION) "." ZEND_TOSTR(V8_MINOR_VERSION));
#ifdef HAVE_NATIVE_ANTLR4
    php_info_print_table_row(2, "Antlr Version", antlr4::RuntimeMetaData::VERSION.c_str());
#else
    php_info_print_table_row(2, "Antlr Version", "4.7.1 (JavaScript Runtime)");
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
    NULL,
    PHP_MINIT(openrasp),
    PHP_MSHUTDOWN(openrasp),
    PHP_RINIT(openrasp),
    PHP_RSHUTDOWN(openrasp),
    PHP_MINFO(openrasp),
    PHP_OPENRASP_VERSION,
    STANDARD_MODULE_PROPERTIES};

#ifdef COMPILE_DL_OPENRASP
ZEND_GET_MODULE(openrasp)
#endif

static bool make_openrasp_root_dir(TSRMLS_D)
{
    char *path = openrasp_ini.root_dir;
    if (!path || !IS_ABSOLUTE_PATH(path, strlen(path)))
    {
        return false;
    }
    path = expand_filepath(path, nullptr TSRMLS_CC);
    if (!path || strnlen(path, 2) == 1)
    {
        efree(path);
        return false;
    }
    std::string root_dir(path);
    std::string default_slash(1, DEFAULT_SLASH);
    efree(path);
    std::vector<std::string> sub_dir_list{
        "assets",
        "conf",
        "plugins",
        "locale",
        "logs" + default_slash + ALARM_LOG_DIR_NAME,
        "logs" + default_slash + POLICY_LOG_DIR_NAME,
        "logs" + default_slash + PLUGIN_LOG_DIR_NAME,
        "logs" + default_slash + RASP_LOG_DIR_NAME};
    for (auto dir : sub_dir_list)
    {
        std::string path(root_dir + DEFAULT_SLASH + dir);
        if (!recursive_mkdir(path.c_str(), path.length(), 0777 TSRMLS_CC))
        {
            return false;
        }
    }
#ifdef HAVE_GETTEXT
    if (nullptr != setlocale(LC_ALL, openrasp_ini.locale ? openrasp_ini.locale : ""))
    {
        std::string locale_path(root_dir + DEFAULT_SLASH + "locale" + DEFAULT_SLASH);
        if (!bindtextdomain(GETTEXT_PACKAGE, locale_path.c_str()))
        {
            openrasp_error(E_WARNING, CONFIG_ERROR, _("bindtextdomain() failed: %s"), strerror(errno));
        }
        if (!textdomain(GETTEXT_PACKAGE))
        {
            openrasp_error(E_WARNING, CONFIG_ERROR, _("textdomain() failed: %s"), strerror(errno));
        }
    }
    else
    {
        openrasp_error(E_WARNING, CONFIG_ERROR, _("Unable to set OpenRASP locale to %s"), openrasp_ini.locale);
    }
#endif
    return true;
}