#include "openrasp_hook.h"
#include "openrasp_ini.h"
#include "openrasp_inject.h"
#include "hook/openrasp_array.h"
#include "hook/openrasp_sql.h"
#include "hook/openrasp_file.h"
#include "hook/command.h"
#include "hook/include.h"
#include "hook/fileUpload.h"
#include "hook/ssrf.h"
#include "hook/directory.h"
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
        if (!PG(http_globals)[pairs[index].id] && !zend_is_auto_global(pairs[index].name, strlen(pairs[index].name) TSRMLS_CC))
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
    char *block_url = openrasp_ini.block_url;
    char *request_id = OPENRASP_INJECT_G(request_id);
    /* 当 headers 已经输出时，只能在 body 中插入 script 进行重定向 */
    if (SG(headers_sent))
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
    else
    {
        char *redirect_header = nullptr;
        int redirect_header_len = 0;
        redirect_header_len = spprintf(&redirect_header, 0, "Location: %s?request_id=%s", block_url, request_id);
        if (redirect_header)
        {
            sapi_header_line header;
            header.line = redirect_header;
            header.line_len = redirect_header_len;
            header.response_code = 302;
            sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
        }
        efree(redirect_header);
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

#define HOOK_CALLABLE(name)                                \
    OPENRASP_HOOK_FUNCTION(name)                           \
    {                                                      \
        pre_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU);      \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
    }

HOOK_CALLABLE(array_diff_ukey);
HOOK_CALLABLE(array_filter);
HOOK_CALLABLE(array_map);
HOOK_CALLABLE(array_walk);
HOOK_CALLABLE(uasort);
HOOK_CALLABLE(uksort);
HOOK_CALLABLE(usort);

inline void hook_reflectionfunction(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *name;
    zval *closure = NULL;
    char *lcname;
    zend_function *fptr;
    char *name_str;
    int name_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name_str, &name_len) == SUCCESS)
    {
        char *nsname;

        lcname = zend_str_tolower_dup(name_str, name_len);

        /* Ignore leading "\" */
        nsname = lcname;
        if (lcname[0] == '\\')
        {
            nsname = &lcname[1];
            name_len--;
        }

        if (zend_hash_find(EG(function_table), nsname, name_len + 1, (void **)&fptr) == FAILURE)
        {
            efree(lcname);
            return;
        }
        if (openrasp_check_callable_black(nsname, name_len TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            ZVAL_STRING(attack_params, nsname, 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            char *message_str = NULL;
            spprintf(&message_str, 0, _("Webshell detected: using %s"), nsname);
            ZVAL_STRING(plugin_message, message_str, 1);
            efree(message_str);
            openrasp_buildin_php_risk_handle(1, "callable", 90, attack_params, plugin_message TSRMLS_CC);
        }
        efree(lcname);
    }
    else
    {
        return;
    }
}

#define HOOK_REFLECTION(name, scope)                               \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                         \
    {                                                              \
        hook_reflectionfunction(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);         \
    }
HOOK_REFLECTION(__construct, reflectionfunction);

#define HOOK_SQL_EX(name, scope, server)                                                           \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                                                         \
    {                                                                                              \
        pre_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char *>(#server));  \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                         \
        post_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char *>(#server)); \
    }
HOOK_SQL_EX(exec, sqlite3, sqlite);
HOOK_SQL_EX(query, sqlite3, sqlite);
HOOK_SQL_EX(querySingle, sqlite3, sqlite);
HOOK_SQL_EX(mysqli, mysqli, mysql);
HOOK_SQL_EX(real_connect, mysqli, mysql);
HOOK_SQL_EX(query, mysqli, mysql);

#define HOOK_SQL(name, server)                                                      \
    OPENRASP_HOOK_FUNCTION(name)                                                    \
    {                                                                               \
        pre_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char *>(#server));  \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                          \
        post_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char *>(#server)); \
    }
HOOK_SQL(mysql_connect, mysql);
HOOK_SQL(mysql_pconnect, mysql);
HOOK_SQL(mysql_query, mysql);
HOOK_SQL(mysqli_connect, mysql);
HOOK_SQL(mysqli_real_connect, mysql);
HOOK_SQL(mysqli_query, mysql);
HOOK_SQL(mysqli_real_query, mysql);
HOOK_SQL(pg_connect, pgsql);
HOOK_SQL(pg_pconnect, pgsql);
HOOK_SQL(pg_query, pgsql);
HOOK_SQL(pg_send_query, pgsql);
HOOK_SQL(pg_get_result, pgsql);

#define HOOK_PDO_EX(name, scope)                                      \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                            \
    {                                                                 \
        pre_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU);  \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);            \
        post_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
    }
HOOK_PDO_EX(query, pdo);
HOOK_PDO_EX(exec, pdo);
HOOK_PDO_EX(__construct, pdo);

/**
 * 文件相关hook点
 */
#define HOOK_FILE(name)                                    \
    OPENRASP_HOOK_FUNCTION(name)                           \
    {                                                      \
        hook_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU);     \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
    }
HOOK_FILE(file);
HOOK_FILE(readfile);
HOOK_FILE(file_get_contents);
HOOK_FILE(file_put_contents);
HOOK_FILE(fopen);
HOOK_FILE(copy);

#define HOOK_FILE_EX(name, scope)                                     \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                            \
    {                                                                 \
        hook_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);            \
    }
HOOK_FILE_EX(__construct, splfileobject);

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
    OPENRASP_HOOK(array_diff_ukey);
    OPENRASP_HOOK(array_filter);
    OPENRASP_HOOK(array_map);
    OPENRASP_HOOK(array_walk);
    OPENRASP_HOOK(uasort);
    OPENRASP_HOOK(uksort);
    OPENRASP_HOOK(usort);
    OPENRASP_HOOK(passthru);
    OPENRASP_HOOK(system);
    OPENRASP_HOOK(exec);
    OPENRASP_HOOK(shell_exec);
    OPENRASP_HOOK(proc_open);
    OPENRASP_HOOK(popen);
    OPENRASP_HOOK(pcntl_exec);
    OPENRASP_HOOK(assert);
    OPENRASP_HOOK(file);
    OPENRASP_HOOK(readfile);
    OPENRASP_HOOK(file_get_contents);
    OPENRASP_HOOK(file_put_contents);
    OPENRASP_HOOK(fopen);
    OPENRASP_HOOK(copy);
    OPENRASP_HOOK(mysql_connect);
    OPENRASP_HOOK(mysql_pconnect);
    OPENRASP_HOOK(mysql_query);
    OPENRASP_HOOK(mysqli_connect);
    OPENRASP_HOOK(mysqli_real_connect);
    OPENRASP_HOOK(mysqli_query);
    OPENRASP_HOOK(mysqli_real_query);
    OPENRASP_HOOK(pg_connect);
    OPENRASP_HOOK(pg_pconnect);
    OPENRASP_HOOK(pg_query);
    OPENRASP_HOOK(pg_send_query);
    OPENRASP_HOOK(pg_get_result);
    OPENRASP_HOOK(move_uploaded_file);
    OPENRASP_HOOK(curl_exec);
    OPENRASP_HOOK(dir);
    OPENRASP_HOOK(opendir);
    OPENRASP_HOOK(scandir);
    OPENRASP_HOOK_EX(__construct, splfileobject);
    OPENRASP_HOOK_EX(exec, sqlite3);
    OPENRASP_HOOK_EX(query, sqlite3);
    OPENRASP_HOOK_EX(querySingle, sqlite3);
    OPENRASP_HOOK_EX(mysqli, mysqli);
    OPENRASP_HOOK_EX(real_connect, mysqli);
    OPENRASP_HOOK_EX(query, mysqli);
    OPENRASP_HOOK_EX(query, pdo);
    OPENRASP_HOOK_EX(exec, pdo);
    OPENRASP_HOOK_EX(__construct, pdo);
    OPENRASP_HOOK_EX(__construct, reflectionfunction);
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
