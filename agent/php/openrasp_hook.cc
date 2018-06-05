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

#include "openrasp_hook.h"
#include "openrasp_ini.h"
#include "openrasp_inject.h"
#include <new>
#include <vector>

static std::vector<hook_handler_t> global_hook_handlers;

void register_hook_handler(hook_handler_t hook_handler)
{
    global_hook_handlers.push_back(hook_handler);
}

typedef struct _track_vars_pair_t
{
    int id;
    const char *name;
} track_vars_pair;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_hook)

bool openrasp_zval_in_request(zval *item TSRMLS_DC)
{
    static const track_vars_pair pairs[] = {{TRACK_VARS_POST, "_POST"},
                                            {TRACK_VARS_GET, "_GET"},
                                            {TRACK_VARS_COOKIE, "_COOKIE"}};
    int size = sizeof(pairs) / sizeof(pairs[0]);
    zend_string *skey;
    zval *val;
    for (int index = 0; index < size; ++index)
    {
        zend_string *name = zend_string_init(pairs[index].name, strlen(pairs[index].name), 0);
        if (Z_TYPE(PG(http_globals)[pairs[index].id]) != IS_ARRAY 
        && !zend_is_auto_global(name TSRMLS_CC)
        && Z_TYPE(PG(http_globals)[pairs[index].id]) != IS_ARRAY)
        {
            zend_string_release(name);
            return false;
        }
        HashTable *ht = Z_ARRVAL(PG(http_globals)[pairs[index].id]);
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, skey, val) {
            if (Z_STR_P(item) == Z_STR_P(val))
            {
                zend_string_release(name);
                return true;
            }
        } ZEND_HASH_FOREACH_END();
        zend_string_release(name);
    }
    return false;
}

void openrasp_buildin_php_risk_handle(zend_bool is_block, const char *type, int confidence, zval *params, zend_string *message TSRMLS_DC)
{
    zval params_result;
    array_init(&params_result);
    add_assoc_long(&params_result,   "plugin_confidence", confidence);
    add_assoc_zval(&params_result,   "attack_params",     params);
    add_assoc_str(&params_result, "attack_type",       zend_string_init(type, strlen(type), 0));
    add_assoc_str(&params_result, "plugin_message",    message);
    const char *intercept_state = is_block ? "block" : "log";
    add_assoc_str(&params_result, "intercept_state",   zend_string_init(intercept_state, strlen(intercept_state), 0));
    add_assoc_str(&params_result, "plugin_name",       zend_string_init("php_builtin_plugin", strlen("php_builtin_plugin"), 0));
    alarm_info(&params_result TSRMLS_CC);
    zval_ptr_dtor(&params_result);
    if (is_block)
    {
        handle_block(TSRMLS_C);
    }
}

bool openrasp_check_type_ignored(const char *item_name, uint item_name_length TSRMLS_DC)
{
    return openrasp_ini.hooks_ignore.find(item_name) != openrasp_ini.hooks_ignore.end();
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length TSRMLS_DC)
{
    return openrasp_ini.callable_blacklists.find(item_name) != openrasp_ini.callable_blacklists.end();
}

void handle_block(TSRMLS_D)
{
    int status = php_output_get_status(TSRMLS_C);
    if (status & PHP_OUTPUT_WRITTEN) {
        php_output_discard_all(TSRMLS_C);
    }
    char *block_url = openrasp_ini.block_url;
    char *request_id = OPENRASP_INJECT_G(request_id);
    if (!SG(headers_sent))
    {
        char *redirect_header = nullptr;
        int redirect_header_len = 0;
        redirect_header_len = spprintf(&redirect_header, 0, "Location: %s?request_id=%s", block_url, request_id);
        if (redirect_header)
        {
            sapi_header_line header;
            header.line = redirect_header;
            header.line_len = redirect_header_len;
            header.response_code = openrasp_ini.block_status_code;
            sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
        }
        efree(redirect_header);
    }
    /* body 中插入 script 进行重定向 */
    {
        char *redirect_script = nullptr;
        int redirect_script_len = 0;
        redirect_script_len = spprintf(&redirect_script, 0, "</script><script>location.href=\"%s?request_id=%s\"</script>", block_url, request_id);
        if (redirect_script)
        {
            php_output_write(redirect_script, redirect_script_len TSRMLS_CC);
            php_output_flush(TSRMLS_C);
        }
        efree(redirect_script);
    }
    zend_bailout();
}

/**
 * 调用 openrasp_check 提供的方法进行检测
 * 若需要拦截，直接返回重定向信息，并终止请求
 */
void check(const char *type, zval *params TSRMLS_DC)
{
    char result = openrasp_check(type, params TSRMLS_CC);
    zval_ptr_dtor(params);
    if (result)
    {
        handle_block(TSRMLS_C);
    }
}

PHP_GINIT_FUNCTION(openrasp_hook)
{
#ifdef ZTS
    new (openrasp_hook_globals) _zend_openrasp_hook_globals;
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp_hook)
{
#ifdef ZTS
    openrasp_hook_globals->~_zend_openrasp_hook_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp_hook)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_hook, PHP_GINIT(openrasp_hook), PHP_GSHUTDOWN(openrasp_hook));
    
    for (auto& single_handler : global_hook_handlers)
    {
        single_handler(TSRMLS_C);
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_hook)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_hook, PHP_GSHUTDOWN(openrasp_hook));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_hook);
PHP_RSHUTDOWN_FUNCTION(openrasp_hook);
