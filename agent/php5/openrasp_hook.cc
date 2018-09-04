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
#include "openrasp_utils.h"
#include "openrasp_inject.h"
#include "openrasp_v8.h"
#include <new>
#include <vector>
#include <map>

extern "C"
{
#include "ext/standard/php_fopen_wrappers.h"
}

static hook_handler_t global_hook_handlers[512];
static size_t global_hook_handlers_len = 0;

std::map<OpenRASPCheckType, const char *> CheckTypeNameMap =
    {
        {CALLABLE, "callable"},
        {COMMAND, "command"},
        {DIRECTORY, "directory"},
        {READ_FILE, "readFile"},
        {WRITE_FILE, "writeFile"},
        {COPY, "copy"},
        {RENAME, "rename"},
        {FILE_UPLOAD, "fileUpload"},
        {INCLUDE, "include"},
        {DB_CONNECTION, "dbConnection"},
        {SQL, "sql"},
        {SQL_SLOW_QUERY, "sqlSlowQuert"},
        {SQL_PREPARED, "sqlPrepared"},
        {SSRF, "ssrf"},
        {WEBSHELL_EVAL, "wenshell_eval"},
        {WEBSHELL_COMMAND, "wenshell_command"},
        {WEBSHELL_FILE_PUT_CONTENTS, "webshell_file_put_contents"}};

void register_hook_handler(hook_handler_t hook_handler)
{
    global_hook_handlers[global_hook_handlers_len++] = hook_handler;
}

typedef struct _track_vars_pair_t
{
    int id;
    const char *name;
} track_vars_pair;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_hook)

struct scheme_cmp { 
    bool operator() (const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

//return value estrdup
char *openrasp_real_path(char *filename, int filename_len, zend_bool use_include_path, wrapper_operation w_op TSRMLS_DC)
{
    static const std::map<std::string, int, scheme_cmp> opMap = 
    {
        {"http", READING},
        {"https", READING},
        {"ftp", READING | WRITING | APPENDING},
        {"ftps", READING | WRITING | APPENDING},
        {"php", READING | WRITING | APPENDING | SIMULTANEOUSRW},
        {"zlib", READING | WRITING | APPENDING},
        {"bzip2", READING | WRITING | APPENDING},
        {"zlib", READING},
        {"data", READING},
        {"phar", READING | WRITING | SIMULTANEOUSRW},
        {"ssh2", READING | WRITING | SIMULTANEOUSRW},
        {"rar", READING},
        {"ogg", READING | WRITING | APPENDING},
        {"expect", READING | WRITING | APPENDING}
    };
    char *resolved_path = nullptr;
    resolved_path = php_resolve_path(filename, filename_len, use_include_path ? PG(include_path) : NULL TSRMLS_CC);
    if (nullptr == resolved_path)
    {
        const char *p = fetch_url_scheme(filename);
        if (nullptr !=p)
        {
            std::string scheme(filename, p - filename);
            php_stream_wrapper *wrapper;
            wrapper = php_stream_locate_url_wrapper(filename, nullptr, STREAM_LOCATE_WRAPPERS_ONLY TSRMLS_CC);
            if (wrapper && wrapper->wops)
            {
                if (w_op & RENAMESRC || w_op & RENAMEDEST)
                {
                    if (wrapper->wops->rename)
                    {
                        resolved_path = estrdup(filename);    
                    }
                }
                else if ((w_op & OPENDIR))
                {
                    if (wrapper->wops->dir_opener)
                    {
                        resolved_path = estrdup(filename);    
                    }
                }
                else
                {
                    auto it = opMap.find(scheme);
                    if (it != opMap.end() && w_op & it->second)
                    {
                        resolved_path = estrdup(filename);
                    }
                }
            }
        }
        else
        {
            char expand_path[MAXPATHLEN];
            char real_path[MAXPATHLEN];
            expand_filepath(filename, expand_path TSRMLS_CC);
            if (VCWD_REALPATH(expand_path, real_path))
            {
                if (w_op & OPENDIR || w_op & RENAMESRC)
                {
                    //skip
                }
                else
                {
                    resolved_path = estrdup(expand_path);
                }
            }
            else
            {
                if (w_op & WRITING || w_op & RENAMEDEST)
                {
                    resolved_path = estrdup(expand_path);
                }
            }
        }
    }
    return resolved_path;
}

bool openrasp_zval_in_request(zval *item TSRMLS_DC)
{
    static const track_vars_pair pairs[] = {{TRACK_VARS_POST, "_POST"},
                                            {TRACK_VARS_GET, "_GET"},
                                            {TRACK_VARS_COOKIE, "_COOKIE"}};
    int size = sizeof(pairs) / sizeof(pairs[0]);
    for (int index = 0; index < size; ++index)
    {
        if (!PG(http_globals)[pairs[index].id] && !zend_is_auto_global(pairs[index].name, strlen(pairs[index].name) TSRMLS_CC) && Z_TYPE_P(PG(http_globals)[pairs[index].id]) != IS_ARRAY)
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

void openrasp_buildin_php_risk_handle(zend_bool is_block, OpenRASPCheckType type, int confidence, zval *params, zval *message TSRMLS_DC)
{
    zval *params_result = nullptr;
    MAKE_STD_ZVAL(params_result);
    array_init(params_result);
    add_assoc_string(params_result, "intercept_state", const_cast<char *>(is_block ? "block" : "log"), 1);
    add_assoc_string(params_result, "attack_type", (char *)CheckTypeNameMap.at(type), 1);
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

bool openrasp_check_type_ignored(OpenRASPCheckType check_type TSRMLS_DC)
{
    //TODO 白名单
    return false;
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length TSRMLS_DC)
{
    return openrasp_ini.callable_blacklists.find({item_name, item_name_length}) != openrasp_ini.callable_blacklists.end();
}

void handle_block(TSRMLS_D)
{
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 3)
    if (OG(ob_nesting_level) && (OG(active_ob_buffer).status || OG(active_ob_buffer).erase))
    {
        php_end_ob_buffer(0, 0 TSRMLS_CC);
    }
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 4)
    int status = php_output_get_status(TSRMLS_C);
    if (status & PHP_OUTPUT_WRITTEN)
    {
        php_output_discard(TSRMLS_C);
    }
#else
#error "Unsupported PHP version, please contact OpenRASP team for more information"
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
        redirect_script_len = spprintf(&redirect_script, 0, "</script><script>location.href=\"%s?request_id=%s\"</script>\n", block_url, request_id);
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
void check(OpenRASPCheckType check_type, zval *params TSRMLS_DC)
{
    const char * type = CheckTypeNameMap.at(check_type);
    char result = openrasp::openrasp_check(type, params TSRMLS_CC);
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
    ZEND_INIT_MODULE_GLOBALS(openrasp_hook, PHP_GINIT(openrasp_hook), PHP_GSHUTDOWN(openrasp_hook));

    for (size_t i = 0; i < global_hook_handlers_len; i++)
    {
        global_hook_handlers[i](TSRMLS_C);
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
