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
#include "ext/pcre/php_pcre.h"
#include "ext/standard/file.h"
#include "Zend/zend_builtin_functions.h"
}
#include <string>
#ifdef PHP_WIN32
#include "win32/time.h"
#include <windows.h>
#if defined(HAVE_IPHLPAPI_WS2)
#include <winsock2.h>
#include <iphlpapi.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#endif
#elif defined(NETWARE)
#include <sys/timeval.h>
#include <sys/time.h>
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <net/if.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

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
    auto trace = format_debug_backtrace_str(TSRMLS_C);
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

long fetch_time_offset()
{
    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);
    return lt.tm_gmtoff;
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
                    plugins.push_back(use_abs_path ? (dir_abs + std::string(1, DEFAULT_SLASH) + std::string(ent->d_name)): std::string(ent->d_name));
                }
            }
        }
        closedir(dir);
    }
}

bool same_day_in_current_timezone(long src, long target, long offset)
{
    long day = 24 * 60 * 60;
    return ((src + offset) / day == (target + offset) / day);
}

zend_string *openrasp_format_date(char *format, int format_len, time_t ts)
{
    char buffer[128];
    struct tm *tm_info;

    time(&ts);
    tm_info = localtime(&ts);

    strftime(buffer, 64, format, tm_info);
    return zend_string_init(buffer, strlen(buffer), 0);
}

void openrasp_pcre_match(zend_string *regex, zend_string *subject, zval *return_value)
{
    pcre_cache_entry *pce;
    zval *subpats = NULL;
    zend_long flags = 0;
    zend_long start_offset = 0;
    int global = 0;

    if (ZEND_SIZE_T_INT_OVFL(ZSTR_LEN(subject)))
    {
        RETURN_FALSE;
    }

    if ((pce = pcre_get_compiled_regex_cache(regex)) == NULL)
    {
        RETURN_FALSE;
    }

    pce->refcount++;
    php_pcre_match_impl(pce, ZSTR_VAL(subject), (int)ZSTR_LEN(subject), return_value, subpats,
                        global, 0, flags, start_offset);
    pce->refcount--;
}

long get_file_st_ino(std::string filename)
{
    struct stat sb;
    if (VCWD_STAT(filename.c_str(), &sb) == 0 && (sb.st_mode & S_IFREG) != 0)
    {
        return (long)sb.st_ino;
    }
    return 0;
}

void fetch_if_addrs(std::map<std::string, std::string> &if_addr_map)
{
#if defined(PHP_WIN32) && defined(HAVE_IPHLPAPI_WS2)
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL)
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL)
        {
            openrasp_error(E_WARNING, LOG_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
        }
    }
    if (pAdapterInfo != NULL && (dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            if_addr_map.insert(std::pair<std::string, std::string>(pAdapter->Description, pAdapter->IpAddressList.IpAddress.String));
            pAdapter = pAdapter->Next;
        }
        FREE(pAdapterInfo);
    }
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs() error: %s"), strerror(errno));
    }
    else
    {
        int n, s;
        char host[NI_MAXHOST];
        for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
        {
            if (ifa->ifa_addr == NULL)
            {
                continue;
            }
            if ((strcmp("lo", ifa->ifa_name) == 0) ||
                !(ifa->ifa_flags & (IFF_RUNNING)))
            {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0)
                {
                    openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs error: getnameinfo failed - %s."), gai_strerror(s));
                }
                if_addr_map.insert(std::pair<std::string, std::string>(ifa->ifa_name, host));
            }
        }
        freeifaddrs(ifaddr);
    }
#endif
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