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

ZEND_DECLARE_MODULE_GLOBALS(openrasp_output_detect)

static void _check_header_content_type_if_html(void *data, void *arg TSRMLS_DC);
static int _detect_param_occur_in_html_output(const char *param, OpenRASPActionType action TSRMLS_DC);
static bool _gpc_parameter_filter(const zval *param TSRMLS_DC);
static bool _is_content_type_html(TSRMLS_D);

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)

void openrasp_detect_output(INTERNAL_FUNCTION_PARAMETERS)
{
    OUTPUT_G(output_detect) = true;
    char *input;
    int input_len;
    long mode;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &input, &input_len, &mode) == FAILURE)
    {
        RETVAL_FALSE;
    }
    if (_is_content_type_html(TSRMLS_C))
    {
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_USER_INPUT);
        int status = _detect_param_occur_in_html_output(input, action TSRMLS_CC);
        if (status == SUCCESS)
        {
            status = (AC_BLOCK == action) ? SUCCESS : FAILURE;
        }
        if (status == SUCCESS)
        {
            reset_response(TSRMLS_C);
            RETVAL_STRING("", 1);
        }
    }
    RETVAL_STRINGL(input, input_len, 1);
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
    if (_is_content_type_html(TSRMLS_C) &&
        (output_context->op & PHP_OUTPUT_HANDLER_START) &&
        (output_context->op & PHP_OUTPUT_HANDLER_FINAL))
    {
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_USER_INPUT);
        status = _detect_param_occur_in_html_output(output_context->in.data, action TSRMLS_CC);
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

static void _check_header_content_type_if_html(void *data, void *arg TSRMLS_DC)
{
    bool *is_html = static_cast<bool *>(arg);
    if (*is_html)
    {
        sapi_header_struct *sapi_header = (sapi_header_struct *)data;
        static const char *suffix = "Content-type";
        char *header = (char *)(sapi_header->header);
        size_t header_len = strlen(header);
        size_t suffix_len = strlen(suffix);
        if (header_len > suffix_len &&
            strncmp(suffix, header, suffix_len) == 0 &&
            NULL == strstr(header, "text/html"))
        {
            *is_html = false;
        }
    }
}

static bool _gpc_parameter_filter(const zval *param TSRMLS_DC)
{
    if (Z_TYPE_P(param) == IS_STRING && Z_STRLEN_P(param) > OPENRASP_CONFIG(xss.min_param_length))
    {
        if (openrasp::regex_search(Z_STRVAL_P(param), OPENRASP_CONFIG(xss.filter_regex).c_str()))
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
            if (++count > OPENRASP_CONFIG(xss.max_detection_num))
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
                zval *attack_params = NULL;
                MAKE_STD_ZVAL(attack_params);
                array_init(attack_params);
                add_assoc_string(attack_params, "type", "_GET", 1);
                add_assoc_string(attack_params, "name", const_cast<char *>(name.c_str()), 1);
                add_assoc_string(attack_params, "value", Z_STRVAL_PP(ele_value), 1);
                zval *plugin_message = NULL;
                MAKE_STD_ZVAL(plugin_message);
                char *message_str = NULL;
                spprintf(&message_str, 0, _("Reflected XSS attack detected: parameter: $_GET['%s']"), name.c_str());
                ZVAL_STRING(plugin_message, message_str, 1);
                efree(message_str);
                openrasp_buildin_php_risk_handle(action, XSS_USER_INPUT, 100, attack_params, plugin_message TSRMLS_CC);
                return SUCCESS;
            }
        }
    }
    return status;
}

static bool _is_content_type_html(TSRMLS_D)
{
    bool is_html = true;
    zend_llist_apply_with_argument(&SG(sapi_headers).headers, _check_header_content_type_if_html, &is_html TSRMLS_CC);
    return is_html;
}

static void openrasp_output_detect_init_globals(zend_openrasp_output_detect_globals *openrasp_output_detect_globals)
{
    openrasp_output_detect_globals->output_detect = false;
}

PHP_MINIT_FUNCTION(openrasp_output_detect)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_output_detect, openrasp_output_detect_init_globals, nullptr);
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
