/*
 * Copyright 2017-2021 Baidu Inc.
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
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_inject.h"
#include "openrasp_v8.h"
#include "openrasp_output_detect.h"
#include <new>
#include <map>
#include <algorithm>
#include "agent/shared_config_manager.h"
#include <unordered_map>
#include "openrasp_content_type.h"
#include "openrasp_check_type.h"

extern "C"
{
#include "Zend/zend_exceptions.h"
#include "ext/standard/php_fopen_wrappers.h"
}

using openrasp::OpenRASPContentType;

static const int hookHandlerSize = 256;
static hook_handler_t global_hook_handlers[PriorityType::pTotal][hookHandlerSize] = {0};
static size_t global_hook_handlers_len[PriorityType::pTotal] = {0};
static const std::string COLON_TWO_SLASHES = "://";
static void update_zend_ref_items();

typedef struct _track_vars_pair_t
{
    int id;
    const char *name;
} track_vars_pair;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_hook)

void register_hook_handler(hook_handler_t hook_handler, OpenRASPCheckType type, PriorityType::HookPriority hp)
{
    if (hp < PriorityType::pTotal && global_hook_handlers_len[hp] < hookHandlerSize)
    {
        global_hook_handlers[hp][(global_hook_handlers_len[hp])++] = hook_handler;
    }
}

bool openrasp_zval_in_request(zval *item)
{
    if (nullptr != item)
    {
        auto found = OPENRASP_HOOK_G(zend_ref_items).find(reinterpret_cast<uintptr_t>(Z_COUNTED_P(item)));
        return found != OPENRASP_HOOK_G(zend_ref_items).end();
    }
    return false;
}

bool fetch_name_in_request(zval *item, std::string &name, std::string &type)
{
    if (nullptr != item)
    {
        auto found = OPENRASP_HOOK_G(zend_ref_items).find(reinterpret_cast<uintptr_t>(Z_COUNTED_P(item)));
        if (found != OPENRASP_HOOK_G(zend_ref_items).end())
        {
            static std::unordered_map<int, std::string> id_names =
                {
                    {TRACK_VARS_POST, "_POST"},
                    {TRACK_VARS_GET, "_GET"},
                    {TRACK_VARS_COOKIE, "_COOKIE"}};
            name = found->second.get_name();
            int id = found->second.get_id();
            auto id_found = id_names.find(id);
            if (id_found != id_names.end())
            {
                type = id_found->second;
            }
            return true;
        }
    }
    return false;
}

bool openrasp_check_type_ignored(OpenRASPCheckType check_type)
{
    if (!LOG_G(in_request_process))
    {
        return true;
    }
    if ((1 << check_type) & OPENRASP_HOOK_G(check_type_white_bit_mask))
    {
        return true;
    }
    return false;
}

std::string openrasp_real_path(const char *filename, int length, bool use_include_path, uint32_t w_op)
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
    zend_string *resolved_path = nullptr;
    resolved_path = php_resolve_path(filename, length, use_include_path ? PG(include_path) : nullptr);
    if (nullptr == resolved_path)
    {
        const char *p = determine_scheme_pos(filename);
        if (nullptr != p)
        {
            php_stream_wrapper *wrapper = nullptr;
            wrapper = php_stream_locate_url_wrapper(filename, nullptr, STREAM_LOCATE_WRAPPERS_ONLY);
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
                else if (w_op & UNLINK)
                {
                    if (wrapper->wops->unlink)
                    {
                        resolved_path = zend_string_init(filename, length, 0);
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
                if (w_op & (OPENDIR | RENAMESRC | UNLINK))
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
    else
    {
        if (OPENRASP_CONFIG(plugin.filter))
        {
            if (php_check_open_basedir(ZSTR_VAL(resolved_path)))
            {
                zend_string_release(resolved_path);
                resolved_path = nullptr;
            }
        }
    }
    if (resolved_path)
    {
        result = std::string(ZSTR_VAL(resolved_path), ZSTR_LEN(resolved_path));
        zend_string_release(resolved_path);
    }
    return result;
}

static std::string resolve_request_id(std::string str)
{
    static std::string placeholder = "%request_id%";
    std::string request_id = OPENRASP_G(request).get_id();
    size_t start_pos = 0;
    while ((start_pos = str.find(placeholder, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, placeholder.length(), request_id);
        start_pos += request_id.length();
    }
    return str;
}

void set_location_header(int response_code)
{
    if (!SG(headers_sent))
    {
        std::string location = resolve_request_id("Location: " + OPENRASP_CONFIG(block.redirect_url));
        sapi_header_line header;
        header.line = const_cast<char *>(location.c_str());
        header.line_len = location.length();
        header.response_code = response_code;
        sapi_header_op(SAPI_HEADER_REPLACE, &header);
    }
}

void reset_response()
{
    int response_code = OPENRASP_CONFIG(block.status_code);
    SG(sapi_headers).http_response_code = response_code;
    if (response_code >= 300 && response_code < 400)
    {
        set_location_header(response_code);
    }
}

void block_handle()
{
    if (OUTPUT_G(output_detect))
    {
        return;
    }
    int status = php_output_get_status();
    if (status & PHP_OUTPUT_WRITTEN)
    {
        php_output_discard_all();
    }
    reset_response();

    {
        OpenRASPContentType::ContentType k_type = OpenRASPContentType::ContentType::cNull;
        std::string existing_content_type;
        zend_llist *headers = &SG(sapi_headers).headers;
        if (nullptr != headers && zend_llist_count(headers) > 0)
        {
            for (zend_llist_element *element = headers->head; nullptr != element; element = element->next)
            {
                sapi_header_struct *sapi_header = (sapi_header_struct *)element->data;
                if (nullptr != sapi_header && sapi_header->header_len > 0 &&
                    strncasecmp(sapi_header->header, "content-type", sizeof("content-type") - 1) == 0)
                {
                    existing_content_type = std::string(sapi_header->header);
                    break;
                }
            }
        }
        k_type = OpenRASPContentType::classify_content_type(existing_content_type);
        if (k_type == OpenRASPContentType::ContentType::cNull)
        {
            if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY ||
                zend_is_auto_global_str(ZEND_STRL("_SERVER")))
            {
                zval *z_accept = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRL("HTTP_ACCEPT"));
                if (z_accept)
                {
                    std::string accept(Z_STRVAL_P(z_accept));
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
            std::string body = resolve_request_id(block_content);
            sapi_add_header(const_cast<char *>(content_type.c_str()), content_type.length(), 1);
            php_output_write(body.c_str(), body.length());
            php_output_flush();
        }
    }
    zend_clear_exception();
    zend_bailout();
}

extern int include_or_eval_handler(zend_execute_data *execute_data);
extern int echo_print_handler(zend_execute_data *execute_data);

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

    for (size_t i = 0; i < PriorityType::pTotal; ++i)
    {
        for (size_t j = 0; j < global_hook_handlers_len[i]; ++j)
        {
            global_hook_handlers[i][j]();
        }
    }

    zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, include_or_eval_handler);
    zend_set_user_opcode_handler(ZEND_ECHO, echo_print_handler);
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
        std::string url = OPENRASP_G(request).url.get_complete_url();
        if (!url.empty())
        {
            std::size_t found = url.find(COLON_TWO_SLASHES);
            if (found != std::string::npos)
            {
                OPENRASP_HOOK_G(check_type_white_bit_mask) = openrasp::scm->get_check_type_white_bit_mask(url.substr(found + COLON_TWO_SLASHES.size()));
            }
        }
        if (OPENRASP_HOOK_G(lru).max_size() != OPENRASP_CONFIG(lru.max_size))
        {
            OPENRASP_HOOK_G(lru).reset(OPENRASP_CONFIG(lru.max_size));
        }
        std::vector<OpenRASPCheckType> buindin_check_types = CheckTypeTransfer::instance().get_buildin_check_types();
        for (OpenRASPCheckType check_type : buindin_check_types)
        {
            if (openrasp::scm->get_buildin_check_action(check_type) == AC_IGNORE)
            {
                OPENRASP_HOOK_G(check_type_white_bit_mask) |= (1 << check_type);
            }
        }
    }
    OPENRASP_HOOK_G(origin_pg_error_verbos) = -1;
    update_zend_ref_items();
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp_hook)
{
    OPENRASP_HOOK_G(zend_ref_items).clear();
    return SUCCESS;
}

void update_zend_ref_items()
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
            return;
        }
        zval *val = nullptr;
        zend_string *key = nullptr;
        zend_ulong idx;
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(global), idx, key, val)
        {
            zend_refcounted *val_ref = Z_COUNTED_P(val);
            uintptr_t upt = reinterpret_cast<uintptr_t>(val_ref);
            std::string name;
            if (key != nullptr)
            {
                name = std::string(ZSTR_VAL(key));
            }
            else
            {
                zend_long actual = idx;
                name = std::to_string(actual);
            }
            OPENRASP_HOOK_G(zend_ref_items).insert({upt, openrasp::request::ZendRefItem(pairs[index].id, name)});
        }
        ZEND_HASH_FOREACH_END();
    }
}
