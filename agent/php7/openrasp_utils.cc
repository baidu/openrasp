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
extern "C"
{
#include "php_ini.h"
#include "ext/standard/file.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_builtin_functions.h"
}
#include <string>

void format_debug_backtrace_str(zval *backtrace_str)
{
    zval trace_arr;
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0);
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        std::string buffer;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        zval *ele_value = NULL;        
        ZEND_HASH_FOREACH_VAL(hash_arr, ele_value)
        {
            if (++i > openrasp_ini.log_maxstack)
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
        if (buffer.length() > 0)
        {
            ZVAL_STRINGL(backtrace_str, buffer.c_str(), buffer.length() - 1);
        }
        else
        {
            ZVAL_STRING(backtrace_str, "");
        }
    }
    zval_dtor(&trace_arr);
}

void format_debug_backtrace_arr(zval *backtrace_arr)
{
    zval trace_arr;
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0);
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        zval *ele_value = NULL;
        ZEND_HASH_FOREACH_VAL(hash_arr, ele_value)
        {
            if (++i > openrasp_ini.log_maxstack)
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
            add_next_index_stringl(backtrace_arr, buffer.c_str(), buffer.length());
        }
        ZEND_HASH_FOREACH_END();
    }
    zval_dtor(&trace_arr);
}

void openrasp_error(int type, int error_code, const char *format, ...)
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