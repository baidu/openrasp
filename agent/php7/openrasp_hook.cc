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
#include <map>

extern "C" {
#include "ext/standard/php_fopen_wrappers.h"    
}

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

bool openrasp_zval_in_request(zval *item)
{
    static const track_vars_pair pairs[] = {{TRACK_VARS_POST, "_POST"},
                                            {TRACK_VARS_GET, "_GET"},
                                            {TRACK_VARS_COOKIE, "_COOKIE"}};
    int size = sizeof(pairs) / sizeof(pairs[0]);
    for (int index = 0; index < size; ++index)
    {
        zval *global = &PG(http_globals)[pairs[index].id];
        if (Z_TYPE_P(global) != IS_ARRAY &&
            !zend_is_auto_global_str(const_cast<char *>(pairs[index].name), strlen(pairs[index].name)))
        {
            return false;
        }
        zval *val;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(global), val)
        {
            if (Z_COUNTED_P(item) == Z_COUNTED_P(val))
            {
                return true;
            }
        }
        ZEND_HASH_FOREACH_END();
    }
    return false;
}

void openrasp_buildin_php_risk_handle(zend_bool is_block, const char *type, int confidence, zval *params, zval *message)
{
    zval params_result;
    array_init(&params_result);
    add_assoc_long(&params_result, "plugin_confidence", confidence);
    add_assoc_zval(&params_result, "attack_params", params);
    add_assoc_string(&params_result, "attack_type", const_cast<char *>(type));
    add_assoc_zval(&params_result, "plugin_message", message);
    add_assoc_string(&params_result, "intercept_state", const_cast<char *>(is_block ? "block" : "log"));
    add_assoc_string(&params_result, "plugin_name", const_cast<char *>("php_builtin_plugin"));
    alarm_info(&params_result);
    zval_ptr_dtor(&params_result);
    if (is_block)
    {
        handle_block();
    }
}

bool openrasp_check_type_ignored(const char *item_name, uint item_name_length)
{
    return openrasp_ini.hooks_ignore.find(item_name) != openrasp_ini.hooks_ignore.end();
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length)
{
    return openrasp_ini.callable_blacklists.find(item_name) != openrasp_ini.callable_blacklists.end();
}

struct scheme_cmp { 
    bool operator() (const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

zend_string *openrasp_real_path(char *filename, int length, bool use_include_path, wrapper_operation w_op)
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
    zend_string *resolved_path = nullptr;
    resolved_path = php_resolve_path(filename, length, use_include_path ? PG(include_path) : nullptr);
    if (nullptr == resolved_path)
    {
        const char *p;
        for (p = filename; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++)
            ;
        if ((*p == ':') && (p - filename > 1) && (p[1] == '/') && (p[2] == '/'))
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
                        resolved_path = zend_string_init(filename, length, 0);
                    }
                }
                else if ((w_op & OPENDIR))
                {
                    if (wrapper->wops->dir_opener)
                    {
                        resolved_path = zend_string_init(filename, length, 0);  
                    }
                }
                else
                {
                    auto it = opMap.find(scheme);
                    if (it != opMap.end() && w_op & it->second)
                    {
                        resolved_path = zend_string_init(filename, length, 0);
                    }
                }
            }
        }
        else
        {
            char expand_path[MAXPATHLEN];
            char real_path[MAXPATHLEN];
            expand_filepath(filename, expand_path);
            if (VCWD_REALPATH(expand_path, real_path))
            {
                if (w_op & OPENDIR || w_op & RENAMESRC)
                {
                    //skip
                }
                else
                {
                    resolved_path = zend_string_init(expand_path, strlen(expand_path), 0);
                }
            }
            else
            {
                if (w_op & WRITING || w_op & RENAMEDEST)
                {
                    resolved_path = zend_string_init(expand_path, strlen(expand_path), 0);
                }
            }
        }
    }
    return resolved_path;
}

void handle_block()
{
    int status = php_output_get_status();
    if (status & PHP_OUTPUT_WRITTEN)
    {
        php_output_discard_all();
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
            sapi_header_op(SAPI_HEADER_REPLACE, &header);
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
            php_output_write(redirect_script, redirect_script_len);
            php_output_flush();
        }
        efree(redirect_script);
    }
    zend_bailout();
}

/**
 * 调用 openrasp_check 提供的方法进行检测
 * 若需要拦截，直接返回重定向信息，并终止请求
 */
void check(const char *type, zval *params)
{
    char result = openrasp_check(type, params);
    zval_ptr_dtor(params);
    if (result)
    {
        handle_block();
    }
}

extern int include_or_eval_handler(zend_execute_data *execute_data);

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

    for (auto &single_handler : global_hook_handlers)
    {
        single_handler();
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
