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

typedef struct _track_vars_pair_t
{
    int id;
    const char *name;
} track_vars_pair;

#define REGISTER_HOOK_HANDLER_EX(name, scope, type)                     \
    {                                                                   \
        extern void scope##_##name##_##type##_handler(TSRMLS_D);        \
        global_hook_handlers.insert(scope##_##name##_##type##_handler); \
    }

#define REGISTER_HOOK_HANDLER(name, type)   \
    REGISTER_HOOK_HANDLER_EX(name, global, type)

ZEND_DECLARE_MODULE_GLOBALS(openrasp_hook)

bool openrasp_zval_in_request(zval *item TSRMLS_DC)
{
    static const track_vars_pair pairs[] = {{TRACK_VARS_POST, "_POST"},
                                            {TRACK_VARS_GET, "_GET"},
                                            {TRACK_VARS_COOKIE, "_COOKIE"}};
    int size = sizeof(pairs) / sizeof(pairs[0]);
    for (int index = 0; index < size; ++index)
    {
        if (!PG(http_globals)[pairs[index].id] 
        && !zend_is_auto_global(pairs[index].name, strlen(pairs[index].name) TSRMLS_CC)
        && Z_TYPE_P(PG(http_globals)[pairs[index].id]) != IS_ARRAY)
        {
            return 0;
        }
        HashTable *ht = Z_ARRVAL_P(PG(http_globals)[pairs[index].id]);
        for (zend_hash_internal_pointer_reset(ht);
             zend_hash_has_more_elements(ht) == SUCCESS;
             zend_hash_move_forward(ht))
        {
            zval **ele_value;
            if (zend_hash_get_current_data(ht, (void **)&ele_value) != SUCCESS)
            {
                continue;
            }
            if (item == *ele_value)
            {
                return 1;
            }
        }
    }
    return 0;
}

void openrasp_buildin_php_risk_handle(zend_bool is_block, const char *type, int confidence, zval *params, zval *message TSRMLS_DC)
{
    zval *params_result = nullptr;
    MAKE_STD_ZVAL(params_result);
    array_init(params_result);
    add_assoc_string(params_result, "intercept_state", const_cast<char *>(is_block ? "block" : "log"), 1);
    add_assoc_string(params_result, "attack_type", (char *)type, 1);
    add_assoc_string(params_result, "plugin_name", const_cast<char *>("php_builtin_plugin"), 1);
    add_assoc_long(params_result, "plugin_confidence", confidence);
    add_assoc_zval(params_result, "attack_params", params);
    add_assoc_zval(params_result, "plugin_message", message);
    alarm_info(params_result TSRMLS_CC);
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
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 3)
    if (OG(ob_nesting_level) && (OG(active_ob_buffer).status || OG(active_ob_buffer).erase)) {
        php_end_ob_buffer(0, 0 TSRMLS_CC);
    }
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 4)
    int status = php_output_get_status(TSRMLS_C);
    if (status & PHP_OUTPUT_WRITTEN) {
        php_output_discard(TSRMLS_C);
    }
#else
#  error "Unsupported PHP version, please contact OpenRASP team for more information"
#endif

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
#if PHP_MINOR_VERSION > 3
            php_output_write(redirect_script, redirect_script_len TSRMLS_CC);
            php_output_flush(TSRMLS_C);
#else
            php_body_write(redirect_script, redirect_script_len TSRMLS_CC);
#endif
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
    zval_ptr_dtor(&params);
    if (result)
    {
        handle_block(TSRMLS_C);
    }
}

extern int include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS);

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
    static std::unordered_set<void (*)(TSRMLS_D)> global_hook_handlers;
    ZEND_INIT_MODULE_GLOBALS(openrasp_hook, PHP_GINIT(openrasp_hook), PHP_GSHUTDOWN(openrasp_hook));

    REGISTER_HOOK_HANDLER(array_diff_ukey, callable);
    REGISTER_HOOK_HANDLER(array_filter, callable);
    REGISTER_HOOK_HANDLER(array_map, callable);
    REGISTER_HOOK_HANDLER(array_walk, callable);
    REGISTER_HOOK_HANDLER(uasort, callable);
    REGISTER_HOOK_HANDLER(uksort, callable);
    REGISTER_HOOK_HANDLER(usort, callable);
    REGISTER_HOOK_HANDLER_EX(__construct, reflectionfunction, callable);

    REGISTER_HOOK_HANDLER(passthru, command);
    REGISTER_HOOK_HANDLER(system, command);
    REGISTER_HOOK_HANDLER(exec, command);
    REGISTER_HOOK_HANDLER(shell_exec, command);
    REGISTER_HOOK_HANDLER(proc_open, command);
    REGISTER_HOOK_HANDLER(popen, command);
    REGISTER_HOOK_HANDLER(pcntl_exec, command);

    REGISTER_HOOK_HANDLER(passthru, webshell_command);
    REGISTER_HOOK_HANDLER(system, webshell_command);
    REGISTER_HOOK_HANDLER(exec, webshell_command);
    REGISTER_HOOK_HANDLER(shell_exec, webshell_command);
    REGISTER_HOOK_HANDLER(proc_open, webshell_command);
    REGISTER_HOOK_HANDLER(popen, webshell_command);
    REGISTER_HOOK_HANDLER(pcntl_exec, webshell_command);
    REGISTER_HOOK_HANDLER(assert, webshell_eval);

    REGISTER_HOOK_HANDLER(file, readFile);
    REGISTER_HOOK_HANDLER(readfile, readFile);
    REGISTER_HOOK_HANDLER(file_get_contents, readFile);
    REGISTER_HOOK_HANDLER(file_put_contents, writeFile);
    REGISTER_HOOK_HANDLER(file_put_contents, webshell_file_put_contents);
    REGISTER_HOOK_HANDLER(fopen, readFile);
    REGISTER_HOOK_HANDLER(fopen, writeFile);
    REGISTER_HOOK_HANDLER(copy, writeFile);
    REGISTER_HOOK_HANDLER(copy, readFile);
    REGISTER_HOOK_HANDLER_EX(__construct, splfileobject, readFile);
    REGISTER_HOOK_HANDLER_EX(__construct, splfileobject, writeFile);

    REGISTER_HOOK_HANDLER(mysql_connect, dbConnection);
    REGISTER_HOOK_HANDLER(mysql_pconnect, dbConnection);
    REGISTER_HOOK_HANDLER(mysql_query, sqlSlowQuery);
    REGISTER_HOOK_HANDLER(mysql_query, sql);

    REGISTER_HOOK_HANDLER(mysqli_connect, dbConnection);
    REGISTER_HOOK_HANDLER(mysqli_real_connect, dbConnection);
    REGISTER_HOOK_HANDLER(mysqli_query, sqlSlowQuery);
    REGISTER_HOOK_HANDLER(mysqli_query, sql);
    REGISTER_HOOK_HANDLER(mysqli_prepare, sqlPrepare);
    REGISTER_HOOK_HANDLER(mysqli_real_query, sql);
    REGISTER_HOOK_HANDLER_EX(mysqli, mysqli, dbConnection);
    REGISTER_HOOK_HANDLER_EX(real_connect, mysqli, dbConnection);
    REGISTER_HOOK_HANDLER_EX(query, mysqli, sqlSlowQuery);
    REGISTER_HOOK_HANDLER_EX(query, mysqli, sql);
    REGISTER_HOOK_HANDLER_EX(prepare, mysqli, sqlPrepare);

    REGISTER_HOOK_HANDLER(pg_connect, dbConnection);
    REGISTER_HOOK_HANDLER(pg_pconnect, dbConnection);
    REGISTER_HOOK_HANDLER(pg_query, sqlSlowQuery);
    REGISTER_HOOK_HANDLER(pg_query, sql);
    REGISTER_HOOK_HANDLER(pg_send_query, sql);
    REGISTER_HOOK_HANDLER(pg_get_result, sqlSlowQuery);
    REGISTER_HOOK_HANDLER(pg_prepare, sqlPrepare);

    REGISTER_HOOK_HANDLER_EX(exec, sqlite3, sql);
    REGISTER_HOOK_HANDLER_EX(query, sqlite3, sql);
    REGISTER_HOOK_HANDLER_EX(querySingle, sqlite3, sql);

    REGISTER_HOOK_HANDLER_EX(query, pdo, sqlSlowQuery);
    REGISTER_HOOK_HANDLER_EX(query, pdo, sql);
    REGISTER_HOOK_HANDLER_EX(exec, pdo, sqlSlowQuery);
    REGISTER_HOOK_HANDLER_EX(exec, pdo, sql);
    REGISTER_HOOK_HANDLER_EX(__construct, pdo, dbConnection);
    REGISTER_HOOK_HANDLER_EX(prepare, pdo, sqlPrepare);

    REGISTER_HOOK_HANDLER(move_uploaded_file, fileUpload);

    REGISTER_HOOK_HANDLER(curl_exec, ssrf);

    REGISTER_HOOK_HANDLER(dir, directory);
    REGISTER_HOOK_HANDLER(opendir, directory);
    REGISTER_HOOK_HANDLER(scandir, directory);

    for (auto& single_handler : global_hook_handlers)
    {
        single_handler(TSRMLS_C);
    }

    zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, include_or_eval_handler);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_hook)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_hook, PHP_GSHUTDOWN(openrasp_hook));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_hook);
PHP_RSHUTDOWN_FUNCTION(openrasp_hook);
