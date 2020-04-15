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

static int _detect_param_occur_in_html_output(const char *param, OpenRASPActionType action TSRMLS_DC);
static bool _gpc_parameter_filter(const zval *param TSRMLS_DC);
static const char *get_content_type(TSRMLS_D);
static int check_xss(const char *content, size_t content_length, const char *content_type TSRMLS_DC);
static void check_sensitive_content(const char *content, size_t content_length, const char *content_type TSRMLS_DC);

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)

void openrasp_detect_output(INTERNAL_FUNCTION_PARAMETERS)
{
    OUTPUT_G(output_detect) = true;
    char *content;
    int content_length;
    long mode;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &content, &content_length, &mode) == FAILURE)
    {
        RETVAL_FALSE;
    }
    auto content_type = get_content_type(TSRMLS_C);
    check_sensitive_content(content, content_length, content_type TSRMLS_CC);
    int status = check_xss(content, content_length, content_type TSRMLS_CC);
    if (status == SUCCESS)
    {
        RETVAL_STRING("", 1);
    }
    RETVAL_STRINGL(content, content_length, 1);
}

#else

static php_output_handler *openrasp_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags TSRMLS_DC);
static void openrasp_clean_output_start(const char *name, size_t name_len TSRMLS_DC);
static int openrasp_output_handler(void **nothing, php_output_context *output_context);

static int openrasp_output_handler(void **nothing, php_output_context *output_context)
{
    PHP_OUTPUT_TSRMLS(output_context);
    OUTPUT_G(output_detect) = true;
    int status = FAILURE;
    if ((output_context->op & PHP_OUTPUT_HANDLER_START) &&
        (output_context->op & PHP_OUTPUT_HANDLER_FINAL))
    {
        auto content = output_context->in.data;
        auto content_length = strnlen(output_context->in.data, output_context->in.size);
        auto content_type = get_content_type(TSRMLS_C);
        check_sensitive_content(content, content_length, content_type TSRMLS_CC);
        status = check_xss(content, content_length, content_type TSRMLS_CC);
    }
    return status;
}

static php_output_handler *openrasp_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags TSRMLS_DC)
{
    if (chunk_size)
    {
        return nullptr;
    }
    return php_output_handler_create_internal(handler_name, handler_name_len, openrasp_output_handler, chunk_size, flags TSRMLS_CC);
}

static void openrasp_clean_output_start(const char *name, size_t name_len TSRMLS_DC)
{
    php_output_handler *h;

    if ((h = openrasp_output_handler_init(name, name_len, 0, PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC)))
    {
        php_output_handler_start(h TSRMLS_CC);
    }
}

#endif

static bool _gpc_parameter_filter(const zval *param TSRMLS_DC)
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

static int _detect_param_occur_in_html_output(const char *param, OpenRASPActionType action TSRMLS_DC)
{
    int status = FAILURE;
    if (!PG(http_globals)[TRACK_VARS_GET] &&
        !zend_is_auto_global("_GET", strlen("_GET") TSRMLS_CC) &&
        Z_TYPE_P(PG(http_globals)[TRACK_VARS_GET]) != IS_ARRAY)
    {
        return FAILURE;
    }
    HashTable *ht = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]);
    int count = 0;
    for (zend_hash_internal_pointer_reset(ht);
         zend_hash_has_more_elements(ht) == SUCCESS;
         zend_hash_move_forward(ht))
    {
        char *key;
        ulong idx;
        int type;
        type = zend_hash_get_current_key(ht, &key, &idx, 0);
        if (type == HASH_KEY_NON_EXISTENT)
        {
            continue;
        }
        zval **ele_value;
        if (zend_hash_get_current_data(ht, (void **)&ele_value) != SUCCESS)
        {
            continue;
        }
        if (_gpc_parameter_filter(*ele_value TSRMLS_CC))
        {
            if (++count > OUTPUT_G(max_detection_num))
            {
                continue;
            }
            if (NULL != strstr(param, Z_STRVAL_PP(ele_value)))
            {
                std::string name;
                if (type == HASH_KEY_IS_STRING)
                {
                    name = std::string(key);
                }
                else if (type == HASH_KEY_IS_LONG)
                {
                    long actual = idx;
                    name = std::to_string(actual);
                }
                openrasp::data::XssUserInputObject xss_obj(name, *ele_value);
                openrasp::checker::BuiltinDetector builtin_detector(xss_obj);
                builtin_detector.run();
                return SUCCESS;
            }
        }
    }
    return status;
}

static const char *get_content_type(TSRMLS_D)
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

static int check_xss(const char *content, size_t content_length, const char *content_type TSRMLS_DC)
{
    int status = FAILURE;
    auto type = OpenRASPContentType::classify_content_type(content_type);
    if (OpenRASPContentType::cTextHtml == type || OpenRASPContentType::cNull == type)
    {
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_USER_INPUT);
        status = _detect_param_occur_in_html_output(content, action TSRMLS_CC);
        if (status == SUCCESS)
        {
            status = (AC_BLOCK == action) ? SUCCESS : FAILURE;
        }
        if (status == SUCCESS)
        {
            reset_response(TSRMLS_C);
        }
    }
    return status;
}

static void check_sensitive_content(const char *content, size_t content_length, const char *content_type TSRMLS_DC)
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
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 3)
    php_output_handler_alias_register(ZEND_STRL("openrasp_ob_handler"), openrasp_output_handler_init TSRMLS_CC);
#endif
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_output_detect)
{
    OUTPUT_G(output_detect) = false;
    if (!openrasp_check_type_ignored(XSS_USER_INPUT TSRMLS_CC))
    {
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
        if (php_start_ob_buffer_named("openrasp_ob_handler", 0, 1 TSRMLS_CC) == FAILURE)
        {
            openrasp_error(E_WARNING, RUNTIME_ERROR, _("Failure start OpenRASP output buffering."));
        }
#else
        openrasp_clean_output_start(ZEND_STRL("openrasp_ob_handler") TSRMLS_CC);
#endif
    }
    return SUCCESS;
}
