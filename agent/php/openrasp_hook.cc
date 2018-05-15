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

/**
 * fileupload相关hook点
 */
PRE_HOOK_FUNCTION(move_uploaded_file, fileUpload);

/**
 * directory相关hook点
 */
PRE_HOOK_FUNCTION(dir, directory);
PRE_HOOK_FUNCTION(opendir, directory);
PRE_HOOK_FUNCTION(scandir, directory);

/**
 * command相关hook点
 */
PRE_HOOK_FUNCTION(passthru, command);
PRE_HOOK_FUNCTION(system, command);
PRE_HOOK_FUNCTION(exec, command);
PRE_HOOK_FUNCTION(shell_exec, command);
PRE_HOOK_FUNCTION(proc_open, command);
PRE_HOOK_FUNCTION(popen, command);
PRE_HOOK_FUNCTION(pcntl_exec, command);

PRE_HOOK_FUNCTION(passthru, webshell_command);
PRE_HOOK_FUNCTION(system, webshell_command);
PRE_HOOK_FUNCTION(exec, webshell_command);
PRE_HOOK_FUNCTION(shell_exec, webshell_command);
PRE_HOOK_FUNCTION(proc_open, webshell_command);
PRE_HOOK_FUNCTION(popen, webshell_command);
PRE_HOOK_FUNCTION(pcntl_exec, webshell_command);
PRE_HOOK_FUNCTION(assert, webshell_eval);

/**
 * callable相关hook点
 */
PRE_HOOK_FUNCTION(usort, callable);
PRE_HOOK_FUNCTION(uksort, callable);
PRE_HOOK_FUNCTION(uasort, callable);
PRE_HOOK_FUNCTION(array_walk, callable);
PRE_HOOK_FUNCTION(array_map, callable);
PRE_HOOK_FUNCTION(array_filter, callable);
PRE_HOOK_FUNCTION(array_diff_ukey, callable);

PRE_HOOK_FUNCTION_EX(__construct, reflectionfunction, callable);

/**
 * sql相关hook点
 */
HOOK_FUNCTION_EX(mysqli, mysqli, dbConnection);
HOOK_FUNCTION_EX(real_connect, mysqli, dbConnection);
PRE_HOOK_FUNCTION_EX(query, mysqli, sql);
POST_HOOK_FUNCTION_EX(query, mysqli, sqlSlowQuery);
PRE_HOOK_FUNCTION_EX(exec, sqlite3, sql);
PRE_HOOK_FUNCTION_EX(query, sqlite3, sql);
PRE_HOOK_FUNCTION_EX(querySingle, sqlite3, sql);

HOOK_FUNCTION(mysql_connect, dbConnection);
HOOK_FUNCTION(mysql_pconnect, dbConnection);
PRE_HOOK_FUNCTION(mysql_query, sql);
POST_HOOK_FUNCTION(mysql_query, sqlSlowQuery);

HOOK_FUNCTION(mysqli_connect, dbConnection);
HOOK_FUNCTION(mysqli_real_connect, dbConnection);
PRE_HOOK_FUNCTION(mysqli_query, sql);
POST_HOOK_FUNCTION(mysqli_query, sqlSlowQuery);
PRE_HOOK_FUNCTION(mysqli_real_query, sql);
HOOK_FUNCTION(pg_connect, dbConnection);
HOOK_FUNCTION(pg_pconnect, dbConnection);
PRE_HOOK_FUNCTION(pg_query, sql);
POST_HOOK_FUNCTION(pg_query, sqlSlowQuery);
PRE_HOOK_FUNCTION(pg_send_query, sql);
POST_HOOK_FUNCTION(pg_get_result, sqlSlowQuery);

HOOK_FUNCTION_EX(__construct, pdo, dbConnection);
PRE_HOOK_FUNCTION_EX(query, pdo, sql);
POST_HOOK_FUNCTION_EX(query, pdo, sqlSlowQuery);
PRE_HOOK_FUNCTION_EX(exec, pdo, sql);
POST_HOOK_FUNCTION_EX(exec, pdo, sqlSlowQuery);

/**
 * 文件相关hook点
 */
PRE_HOOK_FUNCTION(file, readFile);
PRE_HOOK_FUNCTION(readfile, readFile);
PRE_HOOK_FUNCTION(file_get_contents, readFile);
PRE_HOOK_FUNCTION(file_put_contents, webshell_file_put_contents);
PRE_HOOK_FUNCTION(file_put_contents, writeFile);
PRE_HOOK_FUNCTION(fopen, readFile);
PRE_HOOK_FUNCTION(fopen, writeFile);
PRE_HOOK_FUNCTION(copy, writeFile);
PRE_HOOK_FUNCTION(copy, readFile);

PRE_HOOK_FUNCTION_EX(__construct, splfileobject, readFile);
PRE_HOOK_FUNCTION_EX(__construct, splfileobject, writeFile);

/**
 * ssrf相关hook点
 */
extern int pre_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args);
extern void post_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args);
OPENRASP_HOOK_FUNCTION(curl_exec, ssrf)
{
    bool type_ignored = openrasp_check_type_ignored(ZEND_STRL("ssrf") TSRMLS_CC);
    zval function_name, opt, origin_url, *args[2];
    int skip_post = 1;
    if (!type_ignored)
    {
        INIT_ZVAL(function_name);
        INIT_ZVAL(opt);
        INIT_ZVAL(origin_url);
        ZVAL_STRING(&function_name, "curl_getinfo", 0); // 不需要 zval_dtor

        skip_post = pre_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, "ssrf", &function_name, &opt, &origin_url, args);
    }
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    if (!type_ignored)
    {
        if (!skip_post)
        {
            post_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, "ssrf", &function_name, &opt, &origin_url, args);
        }

        zval_dtor(&opt);
        zval_dtor(&origin_url);
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
    OPENRASP_HOOK(array_diff_ukey, callable);
    OPENRASP_HOOK(array_filter, callable);
    OPENRASP_HOOK(array_map, callable);
    OPENRASP_HOOK(array_walk, callable);
    OPENRASP_HOOK(uasort, callable);
    OPENRASP_HOOK(uksort, callable);
    OPENRASP_HOOK(usort, callable);
    OPENRASP_HOOK(passthru, command);
    OPENRASP_HOOK(system, command);
    OPENRASP_HOOK(exec, command);
    OPENRASP_HOOK(shell_exec, command);
    OPENRASP_HOOK(proc_open, command);
    OPENRASP_HOOK(popen, command);
    OPENRASP_HOOK(pcntl_exec, command);
    OPENRASP_HOOK(passthru, webshell_command);
    OPENRASP_HOOK(system, webshell_command);
    OPENRASP_HOOK(exec, webshell_command);
    OPENRASP_HOOK(shell_exec, webshell_command);
    OPENRASP_HOOK(proc_open, webshell_command);
    OPENRASP_HOOK(popen, webshell_command);
    OPENRASP_HOOK(pcntl_exec, webshell_command);
    OPENRASP_HOOK(assert, webshell_eval);
    OPENRASP_HOOK(file, readFile);
    OPENRASP_HOOK(readfile, readFile);
    OPENRASP_HOOK(file_get_contents, readFile);
    OPENRASP_HOOK(file_put_contents, writeFile);
    OPENRASP_HOOK(file_put_contents, webshell_file_put_contents);
    OPENRASP_HOOK(fopen, readFile);
    OPENRASP_HOOK(fopen, writeFile);
    OPENRASP_HOOK(copy, writeFile);
    OPENRASP_HOOK(copy, readFile);
    OPENRASP_HOOK(mysql_connect, dbConnection);
    OPENRASP_HOOK(mysql_pconnect, dbConnection);
    OPENRASP_HOOK(mysql_query, sqlSlowQuery);
    OPENRASP_HOOK(mysql_query, sql);
    OPENRASP_HOOK(mysqli_connect, dbConnection);
    OPENRASP_HOOK(mysqli_real_connect, dbConnection);
    OPENRASP_HOOK(mysqli_query, sqlSlowQuery);
    OPENRASP_HOOK(mysqli_query, sql);
    OPENRASP_HOOK(mysqli_real_query, sql);
    OPENRASP_HOOK(pg_connect, dbConnection);
    OPENRASP_HOOK(pg_pconnect, dbConnection);
    OPENRASP_HOOK(pg_query, sqlSlowQuery);
    OPENRASP_HOOK(pg_query, sql);
    OPENRASP_HOOK(pg_send_query, sql);
    OPENRASP_HOOK(pg_get_result, sqlSlowQuery);
    OPENRASP_HOOK(move_uploaded_file, fileUpload);
    OPENRASP_HOOK(curl_exec, ssrf);
    OPENRASP_HOOK(dir, directory);
    OPENRASP_HOOK(opendir, directory);
    OPENRASP_HOOK(scandir, directory);
    OPENRASP_HOOK_EX(__construct, splfileobject, readFile);
    OPENRASP_HOOK_EX(__construct, splfileobject, writeFile);
    OPENRASP_HOOK_EX(exec, sqlite3, sql);
    OPENRASP_HOOK_EX(query, sqlite3, sql);
    OPENRASP_HOOK_EX(querySingle, sqlite3, sql);
    OPENRASP_HOOK_EX(mysqli, mysqli, dbConnection);
    OPENRASP_HOOK_EX(real_connect, mysqli, dbConnection);
    OPENRASP_HOOK_EX(query, mysqli, sqlSlowQuery);
    OPENRASP_HOOK_EX(query, mysqli, sql);
    OPENRASP_HOOK_EX(query, pdo, sqlSlowQuery);
    OPENRASP_HOOK_EX(query, pdo, sql);
    OPENRASP_HOOK_EX(exec, pdo, sqlSlowQuery);
    OPENRASP_HOOK_EX(exec, pdo, sql);
    OPENRASP_HOOK_EX(__construct, pdo, dbConnection);
    OPENRASP_HOOK_EX(__construct, reflectionfunction, callable);
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
