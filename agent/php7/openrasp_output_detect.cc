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

#include "openrasp_output_detect.h"
#include "openrasp_hook.h"
#include "openrasp_ini.h"
#include "agent/shared_config_manager.h"
#include "utils/regex.h"
#include "hook/checker/builtin_detector.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/xss_userinput_object.h"
#include "hook/data/response_object.h"
#include "openrasp_content_type.h"
#include "utils/sampler.h"
#include <mutex>

using namespace openrasp;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_output_detect)

Sampler sampler;
std::mutex mtx;

static void _check_header_content_type_if_html(void *data, void *arg);
static int _detect_param_occur_in_html_output(const char *param, OpenRASPActionType action);
static bool _gpc_parameter_filter(const zval *param);
static const char *get_content_type();
static int check_xss(const char *content, size_t content_length, const char *content_type);
static void check_sensitive_content(const char *content, size_t content_length, const char *content_type);

static php_output_handler *openrasp_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags);
static void openrasp_clean_output_start(const char *name, size_t name_len);
static int openrasp_output_handler(void **nothing, php_output_context *output_context);

static int openrasp_output_handler(void **nothing, php_output_context *output_context)
{
    OUTPUT_G(output_detect) = true;
    int status = FAILURE;
    if ((output_context->op & PHP_OUTPUT_HANDLER_START) &&
        (output_context->op & PHP_OUTPUT_HANDLER_FINAL))
    {
        auto content = output_context->in.data;
        auto content_length = strnlen(output_context->in.data, output_context->in.size);
        auto content_type = get_content_type();
        check_sensitive_content(content, content_length, content_type);
        status = check_xss(content, content_length, content_type);
    }
    return status;
}

static php_output_handler *openrasp_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags)
{
    if (chunk_size)
    {
        return nullptr;
    }
    return php_output_handler_create_internal(handler_name, handler_name_len, openrasp_output_handler, chunk_size, flags);
}

static void openrasp_clean_output_start(const char *name, size_t name_len)
{
    php_output_handler *h;

    if ((h = openrasp_output_handler_init(name, name_len, 0, PHP_OUTPUT_HANDLER_STDFLAGS)))
    {
        php_output_handler_start(h);
    }
}

static bool _gpc_parameter_filter(const zval *param)
{
    if (Z_TYPE_P(param) == IS_STRING && Z_STRLEN_P(param) > OUTPUT_G(min_param_length))
    {
        if (openrasp::regex_search(Z_STRVAL_P(param), OUTPUT_G(filter_regex).c_str()))
        {
            return true;
        }
    }
    return false;
}

static int _detect_param_occur_in_html_output(const char *param, OpenRASPActionType action)
{
    int status = FAILURE;
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_GET]) != IS_ARRAY &&
        !zend_is_auto_global_str(ZEND_STRL("_GET")))
    {
        return FAILURE;
    }
    zval *global = &PG(http_globals)[TRACK_VARS_GET];
    int count = 0;
    zval *val;
    zend_string *key;
    zend_ulong idx;
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(global), idx, key, val)
    {
        if (_gpc_parameter_filter(val))
        {
            if (++count > OUTPUT_G(max_detection_num))
            {
                continue;
            }
            if (NULL != strstr(param, Z_STRVAL_P(val)))
            {
                std::string name;
                if (key != nullptr)
                {
                    name = std::string(ZSTR_VAL(key));
                }
                else
                {
                    zend_long actual = idx;
                    name = std::to_string(actual);
                }
                openrasp::data::XssUserInputObject xss_obj(name, val);
                openrasp::checker::BuiltinDetector builtin_detector(xss_obj);
                builtin_detector.run();
                return SUCCESS;
            }
        }
    }
    ZEND_HASH_FOREACH_END();
    return status;
}

static const char *get_content_type()
{
    zend_llist *headers = &SG(sapi_headers).headers;
    if (nullptr != headers && zend_llist_count(headers) > 0)
    {
        for (zend_llist_element *element = headers->head; nullptr != element; element = element->next)
        {
            sapi_header_struct *sapi_header = (sapi_header_struct *)element->data;
            if (nullptr != sapi_header && sapi_header->header_len > sizeof("content-type:") - 1 &&
                strncasecmp(sapi_header->header, "content-type:", sizeof("content-type:") - 1) == 0)
            {
                return sapi_header->header + sizeof("content-type:") - 1;
            }
        }
    }
    return "";
}

static int check_xss(const char *content, size_t content_length, const char *content_type)
{
    int status = FAILURE;
    auto type = OpenRASPContentType::classify_content_type(content_type);
    if (OpenRASPContentType::cTextHtml == type || OpenRASPContentType::cNull == type)
    {
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_USER_INPUT);
        status = _detect_param_occur_in_html_output(content, action);
        if (status == SUCCESS)
        {
            status = (AC_BLOCK == action) ? SUCCESS : FAILURE;
        }
        if (status == SUCCESS)
        {
            reset_response();
        }
    }
    return status;
}

static void check_sensitive_content(const char *content, size_t content_length, const char *content_type)
{
    sampler.update(OPENRASP_G(config).response.sampler_interval, OPENRASP_G(config).response.sampler_burst);
    if (sampler.check())
    {
        data::ResponseObject data(content, content_length, content_type);
        checker::V8Detector checker(data, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis), false);
        checker.run();
    }
}

PHP_GINIT_FUNCTION(openrasp_output_detect)
{
#ifdef ZTS
    new (openrasp_output_detect_globals) _zend_openrasp_output_detect_globals;
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp_output_detect)
{
#ifdef ZTS
    openrasp_output_detect_globals->~_zend_openrasp_output_detect_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp_output_detect)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_output_detect, PHP_GINIT(openrasp_output_detect), PHP_GSHUTDOWN(openrasp_output_detect));
    php_output_handler_alias_register(ZEND_STRL("openrasp_ob_handler"), openrasp_output_handler_init);
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_output_detect)
{
    OUTPUT_G(output_detect) = false;
    if (!openrasp_check_type_ignored(XSS_USER_INPUT))
    {
        openrasp_clean_output_start(ZEND_STRL("openrasp_ob_handler"));
    }
    return SUCCESS;
}