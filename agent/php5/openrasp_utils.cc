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

#include "openrasp_utils.h"
#include "openrasp_ini.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "openrasp_log.h"
#include "utils/debug_trace.h"

extern "C"
{
#include "php_ini.h"
#include "php_main.h"
#include "php_streams.h"
#include "ext/json/php_json.h"
#include "ext/standard/url.h"
#include "ext/standard/file.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_smart_str.h"
#include "Zend/zend_builtin_functions.h"
}

using openrasp::DebugTrace;

static std::vector<DebugTrace> build_debug_trace(long limit TSRMLS_DC)
{
    zval trace_arr;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
    zend_fetch_debug_backtrace(&trace_arr, 0, 0 TSRMLS_CC);
#else
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0 TSRMLS_CC);
#endif
    std::vector<DebugTrace> array;
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        for (zend_hash_internal_pointer_reset(hash_arr);
             zend_hash_has_more_elements(hash_arr) == SUCCESS;
             zend_hash_move_forward(hash_arr))
        {
            if (++i > limit)
            {
                break;
            }
            zval **ele_value;
            if (zend_hash_get_current_data(hash_arr, (void **)&ele_value) != SUCCESS ||
                Z_TYPE_PP(ele_value) != IS_ARRAY)
            {
                continue;
            }
            DebugTrace trace_item;
            zval **trace_ele;
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("file"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                trace_item.set_file(Z_STRVAL_PP(trace_ele));
            }
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("function"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                trace_item.set_function(Z_STRVAL_PP(trace_ele));
            }
            //line number
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("line"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_LONG)
            {
                trace_item.set_line(Z_LVAL_PP(trace_ele));
            }
            array.push_back(trace_item);
        }
    }
    zval_dtor(&trace_arr);
    return array;
}

std::string format_debug_backtrace_str(TSRMLS_D)
{
    std::vector<DebugTrace> trace = build_debug_trace(OPENRASP_CONFIG(log.maxstack) TSRMLS_CC);
    std::string buffer;
    for (DebugTrace &item : trace)
    {
        buffer.append(item.to_log_string() + "\n");
    }
    if (buffer.length() > 0)
    {
        buffer.pop_back();
    }
    return buffer;
}

void format_debug_backtrace_str(zval *backtrace_str TSRMLS_DC)
{
    auto trace = format_debug_backtrace_str(TSRMLS_C);
    ZVAL_STRINGL(backtrace_str, trace.c_str(), trace.length(), 1);
}

std::vector<std::string> format_source_code_arr(TSRMLS_D)
{
    std::vector<DebugTrace> trace = build_debug_trace(OPENRASP_CONFIG(log.maxstack) TSRMLS_CC);
    std::vector<std::string> array;
    for (DebugTrace &item : trace)
    {
        array.push_back(item.get_source_code());
    }
    return array;
}

void format_source_code_arr(zval *source_code_arr TSRMLS_DC)
{
    auto array = format_source_code_arr(TSRMLS_C);
    for (auto &str : array)
    {
        add_next_index_stringl(source_code_arr, str.c_str(), str.length(), 1);
    }
}

std::vector<std::string> format_debug_backtrace_arr(TSRMLS_D)
{
    std::vector<DebugTrace> trace = build_debug_trace(OPENRASP_CONFIG(plugin.maxstack) TSRMLS_CC);
    std::vector<std::string> array;
    for (DebugTrace &item : trace)
    {
        array.push_back(item.to_plugin_string());
    }
    return array;
}

int recursive_mkdir(const char *path, int len, int mode TSRMLS_DC)
{
    struct stat sb;
    if (VCWD_STAT(path, &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
    {
        return 1;
    }
    char *dirname = estrndup(path, len);
    int dirlen = php_dirname(dirname, len);
    int rst = recursive_mkdir(dirname, dirlen, mode TSRMLS_CC);
    efree(dirname);
    if (rst)
    {
#ifndef PHP_WIN32
        mode_t oldmask = umask(0);
        rst = VCWD_MKDIR(path, mode);
        umask(oldmask);
#else
        rst = VCWD_MKDIR(path, mode);
#endif
        if (rst == 0 || EEXIST == errno)
        {
            return 1;
        }
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("Could not create directory '%s': %s"), path, strerror(errno));
    }
    return 0;
}

const char *fetch_url_scheme(const char *filename)
{
    if (nullptr == filename)
    {
        return nullptr;
    }
    const char *p;
    for (p = filename; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++)
        ;
    if ((*p == ':') && (p - filename > 1) && (p[1] == '/') && (p[2] == '/'))
    {
        return p;
    }
    return nullptr;
}

void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins, std::function<bool(const char *filename)> file_filter, bool use_abs_path)
{
    DIR *dir;
    std::string result;
    struct dirent *ent;
    if ((dir = opendir(dir_abs.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (file_filter)
            {
                if (file_filter(ent->d_name))
                {
                    plugins.push_back(use_abs_path ? (dir_abs + std::string(1, DEFAULT_SLASH) + std::string(ent->d_name)) : std::string(ent->d_name));
                }
            }
        }
        closedir(dir);
    }
}

bool get_entire_file_content(const char *file, std::string &content)
{
    std::ifstream ifs(file, std::ifstream::in | std::ifstream::binary);
    if (ifs.is_open() && ifs.good())
    {
        content = {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
        return true;
    }
    return false;
}

char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey)
{
    zval **origin_zv;
    if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS &&
        Z_TYPE_PP(origin_zv) == IS_STRING)
    {
        return Z_STRVAL_PP(origin_zv);
    }
    return nullptr;
}

std::string json_encode_from_zval(zval *value TSRMLS_DC)
{
    smart_str buf_json = {0};
    php_json_encode(&buf_json, value, 0 TSRMLS_CC);
    if (buf_json.a > buf_json.len)
    {
        buf_json.c[buf_json.len] = '\0';
        buf_json.len++;
    }
    std::string result(buf_json.c);
    smart_str_free(&buf_json);
    return result;
}

char *fetch_request_body(size_t max_len TSRMLS_DC)
{
    php_stream *stream = php_stream_open_wrapper("php://input", "rb", 0, NULL);
    if (!stream)
    {
        return estrdup("");
    }
    char *buf = nullptr;
    int len = php_stream_copy_to_mem(stream, &buf, max_len, 0);
    php_stream_close(stream);
    if (len <= 0 || !buf)
    {
        return estrdup("");
    }
    return buf;
}

bool need_alloc_shm_current_sapi()
{
    static const char *supported_sapis[] = {
        "fpm-fcgi",
        "apache2handler",
        NULL};
    const char **sapi_name;
    if (sapi_module.name)
    {
        for (sapi_name = supported_sapis; *sapi_name; sapi_name++)
        {
            if (strcmp(sapi_module.name, *sapi_name) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

std::string convert_to_header_key(char *key, size_t length)
{
    if (key == nullptr ||
        strncmp(key, "HTTP_", 5) != 0)
    {
        return "";
    }
    std::string result(key + 5, length - 5);
    for (auto &ch : result)
    {
        if (ch == '_')
        {
            ch = '-';
        }
        else
        {
            ch = std::tolower(ch);
        }
    }
    return result;
}

bool openrasp_parse_url(const std::string &origin_url, std::string &scheme, std::string &host, std::string &port)
{
    php_url *url = php_url_parse_ex(origin_url.c_str(), origin_url.length());
    if (url)
    {
        if (url->scheme)
        {
            scheme = std::string(url->scheme);
        }
        if (url->host)
        {
            host = std::string(url->host);
        }
        if (url->port)
        {
            port = std::to_string(url->port);
        }
        php_url_free(url);
        return true;
    }
    return false;
}