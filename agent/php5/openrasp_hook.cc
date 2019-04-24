/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "openrasp_output_detect.h"
#include <new>
#include <unordered_map>
#include "agent/shared_config_manager.h"
#include <algorithm>
#include "openrasp_content_type.h"
#include "openrasp_check_type.h"

extern "C"
{
#include "ext/standard/php_fopen_wrappers.h"
}
using openrasp::OpenRASPContentType;

static hook_handler_t global_hook_handlers[512];
static size_t global_hook_handlers_len = 0;
static const std::string COLON_TWO_SLASHES = "://";

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

const std::string get_check_type_name(OpenRASPCheckType type)
{
    return check_type_transfer->type_to_name(type);
}

std::string openrasp_real_path(char *filename, int filename_len, bool use_include_path, uint32_t w_op TSRMLS_DC)
{
    std::string result;
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
    if (!OPENRASP_CONFIG(plugin.filter))
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
    if (resolved_path)
    {
        result = std::string(resolved_path);
        efree(resolved_path);
    }
    return result;
}

bool openrasp_zval_in_request(zval *item TSRMLS_DC)
{
    std::string var_type;
    return !fetch_name_in_request(item, var_type TSRMLS_CC).empty();
}

std::string fetch_name_in_request(zval *item, std::string &var_type TSRMLS_DC)
{
    std::string name;
    static const track_vars_pair pairs[] = {{TRACK_VARS_POST, "_POST"},
                                            {TRACK_VARS_GET, "_GET"},
                                            {TRACK_VARS_COOKIE, "_COOKIE"}};
    int size = sizeof(pairs) / sizeof(pairs[0]);
    for (int index = 0; index < size; ++index)
    {
        if (!PG(http_globals)[pairs[index].id] && !zend_is_auto_global(pairs[index].name, strlen(pairs[index].name) TSRMLS_CC) && Z_TYPE_P(PG(http_globals)[pairs[index].id]) != IS_ARRAY)
        {
            return name;
        }
        HashTable *ht = Z_ARRVAL_P(PG(http_globals)[pairs[index].id]);
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
            if (item == *ele_value)
            {
                if (type == HASH_KEY_IS_STRING)
                {
                    name = std::string(key);
                }
                else if (type == HASH_KEY_IS_LONG)
                {
                    long actual = idx;
                    name = std::to_string(actual);
                }
            }
        }
        if (!name.empty())
        {
            var_type = std::string(pairs[index].name);
            return name;
        }
    }
    return name;
}

void openrasp_buildin_php_risk_handle(OpenRASPActionType action, OpenRASPCheckType type, int confidence, zval *params,
                                      zval *message TSRMLS_DC)
{
    if (AC_IGNORE == action)
    {
        return;
    }
    zval *params_result = nullptr;
    MAKE_STD_ZVAL(params_result);
    array_init(params_result);
    add_assoc_string(params_result, "intercept_state", const_cast<char *>(action_to_string(action).c_str()), 1);
    add_assoc_string(params_result, "attack_type", const_cast<char *>(get_check_type_name(type).c_str()), 1);
    add_assoc_string(params_result, "plugin_algorithm", const_cast<char *>(get_check_type_name(type).c_str()), 1);
    add_assoc_string(params_result, "plugin_name", const_cast<char *>("php_builtin_plugin"), 1);
    add_assoc_long(params_result, "plugin_confidence", confidence);
    add_assoc_zval(params_result, "attack_params", params);
    add_assoc_zval(params_result, "plugin_message", message);
    LOG_G(alarm_logger).log(LEVEL_INFO, params_result TSRMLS_CC);
    zval_ptr_dtor(&params_result);
    if (AC_BLOCK == action)
    {
        handle_block(TSRMLS_C);
    }
}

bool openrasp_check_type_ignored(OpenRASPCheckType check_type TSRMLS_DC)
{
    if (!LOG_G(in_request_process))
    {
        return true;
    }
    if ((1 << check_type) & OPENRASP_HOOK_G(check_type_white_bit_mask))
    {
        return true;
    }
    if (check_type_transfer->is_buildin_check_type(check_type) &&
        openrasp::scm->get_buildin_check_action(check_type) == AC_IGNORE)
    {
        return true;
    }
    return false;
}

bool openrasp_check_callable_black(const char *item_name, uint item_name_length TSRMLS_DC)
{
    std::vector<std::string> callable_blacklist = OPENRASP_CONFIG(webshell_callable.blacklist);
    return std::find(callable_blacklist.begin(), callable_blacklist.end(), std::string(item_name, item_name_length)) != callable_blacklist.end();
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

void set_location_header(int response_code TSRMLS_DC)
{
    if (!SG(headers_sent))
    {
        std::string location = resolve_request_id("Location: " + OPENRASP_CONFIG(block.redirect_url) TSRMLS_CC);
        sapi_header_line header;
        header.line = const_cast<char *>(location.c_str());
        header.line_len = location.length();
        header.response_code = response_code;
        sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
    }
}

void reset_response(TSRMLS_D)
{
    int response_code = OPENRASP_CONFIG(block.status_code);
    SG(sapi_headers).http_response_code = response_code;
    if (response_code >= 300 && response_code < 400)
    {
        set_location_header(response_code TSRMLS_CC);
    }
}

void handle_block(TSRMLS_D)
{
    if (OUTPUT_G(output_detect))
    {
        return;
    }
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

    reset_response(TSRMLS_C);

    {
        OpenRASPContentType::ContentType k_type = OpenRASPContentType::ContentType::cNull;
        std::string existing_content_type;
        for (zend_llist_element *element = SG(sapi_headers).headers.head; element; element = element->next)
        {
            sapi_header_struct *sapi_header = (sapi_header_struct *)element->data;
            if (sapi_header->header_len > 0 &&
                strncasecmp(sapi_header->header, "content-type", sizeof("content-type") - 1) == 0)
            {
                existing_content_type = std::string(sapi_header->header);
                break;
            }
        }
        k_type = OpenRASPContentType::classify_content_type(existing_content_type);
        if (k_type == OpenRASPContentType::ContentType::cNull)
        {
            if (PG(http_globals)[TRACK_VARS_SERVER] ||
                zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
            {
                zval **z_accept = nullptr;
                if (zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRS("HTTP_ACCEPT"), (void **)&z_accept) == SUCCESS &&
                    Z_TYPE_PP(z_accept) == IS_STRING)
                {
                    std::string accept(Z_STRVAL_PP(z_accept));
                    k_type = OpenRASPContentType::classify_accept(accept);
                }
            }
        }
        std::string content_type;
        std::string block_content;
        switch (k_type)
        {
        case OpenRASPContentType::ContentType::cApplicationJson:
            content_type = "Content-type: application/json";
            block_content = (OPENRASP_CONFIG(block.content_json));
            break;
        case OpenRASPContentType::ContentType::cApplicationXml:
            content_type = "Content-type: application/xml";
            block_content = (OPENRASP_CONFIG(block.content_xml));
            break;
        case OpenRASPContentType::ContentType::cTextXml:
            content_type = "Content-type: text/xml";
            block_content = (OPENRASP_CONFIG(block.content_xml));
            break;
        case OpenRASPContentType::ContentType::cTextHtml:
        case OpenRASPContentType::ContentType::cNull:
        default:
            content_type = "Content-type: text/html";
            block_content = (OPENRASP_CONFIG(block.content_html));
            break;
        }
        if (!block_content.empty())
        {
            std::string body = resolve_request_id(block_content TSRMLS_CC);
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

extern int include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS);
extern int echo_handler(ZEND_OPCODE_HANDLER_ARGS);

PHP_GINIT_FUNCTION(openrasp_hook)
{
#ifdef ZTS
    new (openrasp_hook_globals) _zend_openrasp_hook_globals;
#endif
    openrasp_hook_globals->check_type_white_bit_mask = 0;
    openrasp_hook_globals->lru.reset(OPENRASP_CONFIG(lru.max_size));
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
    zend_set_user_opcode_handler(ZEND_ECHO, echo_handler);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_hook)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_hook, PHP_GSHUTDOWN(openrasp_hook));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_hook)
{
    if (openrasp::scm != nullptr)
    {
        char *url = fetch_outmost_string_from_ht(Z_ARRVAL_P(LOG_G(alarm_logger).get_common_info(TSRMLS_C)), "url");
        if (url)
        {
            std::string url_str(url);
            std::size_t found = url_str.find(COLON_TWO_SLASHES);
            if (found != std::string::npos)
            {
                OPENRASP_HOOK_G(check_type_white_bit_mask) = openrasp::scm->get_check_type_white_bit_mask(url_str.substr(found + COLON_TWO_SLASHES.size()));
            }
        }
        if (OPENRASP_HOOK_G(lru).max_size() != OPENRASP_CONFIG(lru.max_size))
        {
            OPENRASP_HOOK_G(lru).reset(OPENRASP_CONFIG(lru.max_size));
        }
    }
    return SUCCESS;
}
PHP_RSHUTDOWN_FUNCTION(openrasp_hook);
