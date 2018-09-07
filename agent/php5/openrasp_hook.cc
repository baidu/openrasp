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
#include <unordered_map>

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

//return value estrdup
char *openrasp_real_path(char *filename, int filename_len, bool use_include_path, uint32_t w_op TSRMLS_DC)
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
    char *resolved_path = nullptr;
    resolved_path = php_resolve_path(filename, filename_len, use_include_path ? PG(include_path) : nullptr TSRMLS_CC);
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
                        resolved_path = estrdup(filename);
                    }
                }
                else if (w_op & OPENDIR)
                {
                    if (wrapper->wops->dir_opener)
                    {
                        resolved_path = estrdup(filename);
                    }
                }
                else
                {
                    std::string scheme(filename, p - filename);
                    for (auto &ch : scheme)
                    {
                        ch = std::tolower(ch);
                    }
                    auto it = opMap.find(scheme);
                    if (it != opMap.end() && (w_op & it->second))
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
                if (w_op & (OPENDIR | RENAMESRC))
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
                if (w_op & (WRITING | RENAMEDEST))
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
    static const std::string all("all");
    return openrasp_ini.hooks_ignore.find(all) != openrasp_ini.hooks_ignore.end() ||
           openrasp_ini.hooks_ignore.find({item_name, item_name_length}) != openrasp_ini.hooks_ignore.end();
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length TSRMLS_DC)
{
    return openrasp_ini.callable_blacklists.find({item_name, item_name_length}) != openrasp_ini.callable_blacklists.end();
}

static std::string resolve_request_id(std::string str TSRMLS_DC)
{
    static std::string placeholder = "%request_id%";
    std::string request_id = OPENRASP_INJECT_G(request_id);
    size_t start_pos = 0;
    while ((start_pos = str.find(placeholder, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, placeholder.length(), request_id);
        start_pos += request_id.length();
    }
    return str;
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
        if (PG(http_globals)[TRACK_VARS_SERVER] ||
            zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
        {
            zval **z_accept = nullptr;
            if (zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRS("HTTP_ACCEPT"), (void **)&z_accept) == SUCCESS &&
                Z_TYPE_PP(z_accept) == IS_STRING)
            {
                std::string accept(Z_STRVAL_PP(z_accept));
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
#if PHP_MINOR_VERSION > 3
            php_output_write(body.c_str(), body.length() TSRMLS_CC);
            php_output_flush(TSRMLS_C);
#else
            php_body_write(body.c_str(), body.length() TSRMLS_CC);
#endif
        }
    }

    zend_bailout();
}

/**
 * 调用 openrasp_check 提供的方法进行检测
 * 若需要拦截，直接返回重定向信息，并终止请求
 */
void check(const char *type, zval *params TSRMLS_DC)
{
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
