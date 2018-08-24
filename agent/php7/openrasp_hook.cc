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
#include <unordered_map>
#include <algorithm>

extern "C"
{
#include "ext/standard/php_fopen_wrappers.h"
}

static hook_handler_t global_hook_handlers[512];
static size_t global_hook_handlers_len = 0;

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
    return openrasp_ini.hooks_ignore.find("all") != openrasp_ini.hooks_ignore.end() ||
           openrasp_ini.hooks_ignore.find(item_name) != openrasp_ini.hooks_ignore.end();
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length)
{
    return openrasp_ini.callable_blacklists.find(item_name) != openrasp_ini.callable_blacklists.end();
}

zend_string *openrasp_real_path(char *filename, int length, bool use_include_path, uint32_t w_op)
{
    static const std::unordered_map<std::string, uint32_t> opMap = {
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
        {"expect", READING | WRITING | APPENDING}};
    if (!openrasp_ini.plugin_filter)
    {
        w_op |= WRITING;
    }
    zend_string *resolved_path = nullptr;
    resolved_path = php_resolve_path(filename, length, use_include_path ? PG(include_path) : nullptr);
    if (nullptr == resolved_path)
    {
        const char *p = fetch_url_scheme(filename);
        if (nullptr != p)
        {
            php_stream_wrapper *wrapper;
            wrapper = php_stream_locate_url_wrapper(filename, nullptr, STREAM_LOCATE_WRAPPERS_ONLY TSRMLS_CC);
            if (wrapper && wrapper->wops)
            {
                if (w_op & (RENAMESRC | RENAMEDEST))
                {
                    if (wrapper->wops->rename)
                    {
                        resolved_path = zend_string_init(filename, length, 0);
                    }
                }
                else if (w_op & OPENDIR)
                {
                    if (wrapper->wops->dir_opener)
                    {
                        resolved_path = zend_string_init(filename, length, 0);
                    }
                }
                else
                {
                    std::string scheme(filename, p - filename);
                    std::transform(scheme.begin(), scheme.end(), scheme.begin(), std::tolower);
                    auto it = opMap.find(scheme);
                    if (it != opMap.end() && (w_op & it->second))
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
                if (w_op & (OPENDIR | RENAMESRC))
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
                if (w_op & (WRITING | RENAMEDEST))
                {
                    resolved_path = zend_string_init(expand_path, strlen(expand_path), 0);
                }
            }
        }
    }
    return resolved_path;
}

static std::string resolve_request_id(std::string str)
{
    static std::string placeholder = "%request_id%";
    std::string request_id = OPENRASP_INJECT_G(request_id);
    size_t start_pos = 0;
    while ((start_pos = str.find(placeholder, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, placeholder.length(), request_id);
        start_pos += request_id.length();
    }
    return std::move(str);
}

void handle_block()
{
    int status = php_output_get_status();
    if (status & PHP_OUTPUT_WRITTEN)
    {
        php_output_discard_all();
    }

    if (!SG(headers_sent))
    {
        std::string location = resolve_request_id("Location: " + std::string(openrasp_ini.block_redirect_url) TSRMLS_CC);
        sapi_header_line header;
        header.line = const_cast<char *>(location.c_str());
        header.line_len = location.length();
        header.response_code = openrasp_ini.block_status_code;
        sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
    }

    {
        std::string content_type;
        std::string body;
        if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY ||
            zend_is_auto_global_str(ZEND_STRL("_SERVER")))
        {
            zval *z_accept = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRL("HTTP_ACCEPT"));
            if (z_accept)
            {
                std::string accept(Z_STRVAL_P(z_accept));
                if (openrasp_ini.block_content_json &&
                    accept.find("application/json") != std::string::npos)
                {
                    content_type = "Content-type: application/json";
                    body = resolve_request_id(openrasp_ini.block_content_json TSRMLS_CC);
                }
                else if (openrasp_ini.block_content_xml &&
                         accept.find("text/xml") != std::string::npos)
                {
                    content_type = "Content-type: text/xml";
                    body = resolve_request_id(openrasp_ini.block_content_xml TSRMLS_CC);
                }
                else if (openrasp_ini.block_content_xml &&
                         accept.find("application/xml") != std::string::npos)
                {
                    content_type = "Content-type: application/xml";
                    body = resolve_request_id(openrasp_ini.block_content_xml TSRMLS_CC);
                }
            }
        }
        if (body.length() == 0 &&
            openrasp_ini.block_content_html)
        {
            content_type = "Content-type: text/html";
            body = resolve_request_id(openrasp_ini.block_content_html TSRMLS_CC);
        }
        if (body.length() > 0)
        {
            sapi_add_header(const_cast<char *>(content_type.c_str()), content_type.length(), 1);
            php_output_write(body.c_str(), body.length());
            php_output_flush();
        }
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
