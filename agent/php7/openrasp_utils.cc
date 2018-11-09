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

#include "openrasp.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"
extern "C"
{
#include "php_ini.h"
#include "zend_smart_str.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/file.h"
#include "ext/json/php_json.h"
#include "Zend/zend_builtin_functions.h"
}
#include <string>

std::string format_debug_backtrace_str()
{
    zval trace_arr;
    std::string buffer;
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0);
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        zval *ele_value = NULL;
        ZEND_HASH_FOREACH_VAL(hash_arr, ele_value)
        {
            if (++i > OPENRASP_CONFIG(log.maxstack))
            {
                break;
            }
            if (Z_TYPE_P(ele_value) != IS_ARRAY)
            {
                continue;
            }
            zval *trace_ele;
            if ((trace_ele = zend_hash_str_find(Z_ARRVAL_P(ele_value), ZEND_STRL("file"))) != NULL &&
                Z_TYPE_P(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_P(trace_ele), Z_STRLEN_P(trace_ele));
            }
            buffer.push_back('(');
            if ((trace_ele = zend_hash_str_find(Z_ARRVAL_P(ele_value), ZEND_STRL("function"))) != NULL &&
                Z_TYPE_P(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_P(trace_ele), Z_STRLEN_P(trace_ele));
            }
            buffer.push_back(':');
            //line number
            if ((trace_ele = zend_hash_str_find(Z_ARRVAL_P(ele_value), ZEND_STRL("line"))) != NULL &&
                Z_TYPE_P(trace_ele) == IS_LONG)
            {
                buffer.append(std::to_string(Z_LVAL_P(trace_ele)));
            }
            else
            {
                buffer.append("-1");
            }
            buffer.append(")\n");
        }
        ZEND_HASH_FOREACH_END();
    }
    zval_dtor(&trace_arr);
    if (buffer.length() > 0)
    {
        buffer.pop_back();
    }
    return buffer;
}

void format_debug_backtrace_str(zval *backtrace_str)
{
    auto trace = format_debug_backtrace_str();
    ZVAL_STRINGL(backtrace_str, trace.c_str(), trace.length());
}

std::vector<std::string> format_debug_backtrace_arr()
{
    zval trace_arr;
    std::vector<std::string> array;
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0);
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        zval *ele_value = NULL;
        ZEND_HASH_FOREACH_VAL(hash_arr, ele_value)
        {
            if (++i > OPENRASP_CONFIG(plugin.maxstack))
            {
                break;
            }
            if (Z_TYPE_P(ele_value) != IS_ARRAY)
            {
                continue;
            }
            std::string buffer;
            zval *trace_ele;
            if ((trace_ele = zend_hash_str_find(Z_ARRVAL_P(ele_value), ZEND_STRL("file"))) != NULL &&
                Z_TYPE_P(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_P(trace_ele), Z_STRLEN_P(trace_ele));
            }
            if ((trace_ele = zend_hash_str_find(Z_ARRVAL_P(ele_value), ZEND_STRL("function"))) != NULL &&
                Z_TYPE_P(trace_ele) == IS_STRING)
            {
                buffer.push_back('@');
                buffer.append(Z_STRVAL_P(trace_ele), Z_STRLEN_P(trace_ele));
            }
            array.push_back(buffer);
        }
        ZEND_HASH_FOREACH_END();
    }
    zval_dtor(&trace_arr);
    return array;
}

void format_debug_backtrace_arr(zval *backtrace_arr)
{
    auto array = format_debug_backtrace_arr();
    for (auto &str : array)
    {
        add_next_index_stringl(backtrace_arr, str.c_str(), str.length());
    }
}

void openrasp_error(int type, openrasp_error_code error_code, const char *format, ...)
{
    va_list arg;
    char *message = nullptr;
    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    zend_error(type, "[OpenRASP] %d %s", error_code, message);
    efree(message);
}

int recursive_mkdir(const char *path, int len, int mode)
{
    struct stat sb;
    if (VCWD_STAT(path, &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
    {
        return 1;
    }
    char *dirname = estrndup(path, len);
    int dirlen = php_dirname(dirname, len);
    int rst = recursive_mkdir(dirname, dirlen, mode);
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
        openrasp_error(E_WARNING, CONFIG_ERROR, _("Could not create directory '%s': %s"), path, strerror(errno));
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

char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_STRING)
    {
        return Z_STRVAL_P(origin_zv);
    }
    return nullptr;
}

HashTable *fetch_outmost_hashtable_from_ht(HashTable *ht, const char *arKey)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_ARRAY)
    {
        return Z_ARRVAL_P(origin_zv);
    }
    return nullptr;
}

bool fetch_outmost_long_from_ht(HashTable *ht, const char *arKey, long *result)
{
    zval *origin_zv;
    if ((origin_zv = zend_hash_str_find(ht, arKey, strlen(arKey))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_LONG)
    {
        *result = Z_LVAL_P(origin_zv);
        return true;
    }
    return false;
}

bool write_str_to_file(const char *file, std::ios_base::openmode mode, const char *content, size_t content_len)
{
    std::ofstream out_file(file, mode);
    if (out_file.is_open() && out_file.good())
    {
        out_file.write(content, content_len);
        out_file.close();
        return true;
    }
    return false;
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

std::string json_encode_from_zval(zval *value)
{
    smart_str buf_json = {0};
    php_json_encode(&buf_json, value, 0);
    smart_str_0(&buf_json);
    std::string result(ZSTR_VAL(buf_json.s));
    smart_str_free(&buf_json);
    return result;
}
