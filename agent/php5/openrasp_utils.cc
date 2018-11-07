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

#include "openrasp_utils.h"
#include "openrasp_ini.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>

extern "C"
{
#include "php_ini.h"
#include "ext/json/php_json.h"
#include "ext/standard/url.h"
#include "ext/standard/file.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_smart_str.h"
#include "Zend/zend_builtin_functions.h"
}

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
#elif defined(__linux__)
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

std::string format_debug_backtrace_str(TSRMLS_D)
{
    zval trace_arr;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
    zend_fetch_debug_backtrace(&trace_arr, 0, 0 TSRMLS_CC);
#else
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0 TSRMLS_CC);
#endif
    std::string buffer;
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        for (zend_hash_internal_pointer_reset(hash_arr);
             zend_hash_has_more_elements(hash_arr) == SUCCESS;
             zend_hash_move_forward(hash_arr))
        {
            if (++i > OPENRASP_CONFIG(log.maxstack))
            {
                break;
            }
            zval **ele_value;
            if (zend_hash_get_current_data(hash_arr, (void **)&ele_value) != SUCCESS ||
                Z_TYPE_PP(ele_value) != IS_ARRAY)
            {
                continue;
            }
            zval **trace_ele;
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("file"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_PP(trace_ele), Z_STRLEN_PP(trace_ele));
            }
            buffer.push_back('(');
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("function"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_PP(trace_ele), Z_STRLEN_PP(trace_ele));
            }
            buffer.push_back(':');
            //line number
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("line"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_LONG)
            {
                buffer.append(std::to_string(Z_LVAL_PP(trace_ele)));
            }
            else
            {
                buffer.append("-1");
            }
            buffer.append(")\n");
        }
    }
    zval_dtor(&trace_arr);
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

std::vector<std::string> format_debug_backtrace_arr(TSRMLS_D)
{
    zval trace_arr;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
    zend_fetch_debug_backtrace(&trace_arr, 0, 0 TSRMLS_CC);
#else
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0 TSRMLS_CC);
#endif
    std::vector<std::string> array;
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        for (zend_hash_internal_pointer_reset(hash_arr);
             zend_hash_has_more_elements(hash_arr) == SUCCESS;
             zend_hash_move_forward(hash_arr))
        {
            if (++i > OPENRASP_CONFIG(plugin.maxstack))
            {
                break;
            }
            zval **ele_value;
            if (zend_hash_get_current_data(hash_arr, (void **)&ele_value) != SUCCESS ||
                Z_TYPE_PP(ele_value) != IS_ARRAY)
            {
                continue;
            }
            std::string buffer;
            zval **trace_ele;
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("file"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                buffer.append(Z_STRVAL_PP(trace_ele), Z_STRLEN_PP(trace_ele));
            }
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("function"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                buffer.push_back('@');
                buffer.append(Z_STRVAL_PP(trace_ele), Z_STRLEN_PP(trace_ele));
            }
            array.push_back(buffer);
        }
    }
    zval_dtor(&trace_arr);
    return array;
}

void format_debug_backtrace_arr(zval *backtrace_arr TSRMLS_DC)
{
    auto array = format_debug_backtrace_arr(TSRMLS_C);
    for (auto &str : array)
    {
        add_next_index_stringl(backtrace_arr, str.c_str(), str.length(), 1);
    }
}

void openrasp_error(int type, openrasp_error_code code, const char *format, ...)
{
    va_list arg;
    char *message = nullptr;
    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    zend_error(type, "[OpenRASP] %d %s", code, message);
    efree(message);
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
        openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs error: %s"), strerror(errno));
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
            if ((ifa->ifa_flags & (IFF_LOOPBACK)) ||
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

void fetch_hw_addrs(std::vector<std::string> &hw_addrs)
{
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs error: %s"), strerror(errno));
    }
    else
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
            {
                continue;
            }
#if defined(__linux__)
            if ((ifa->ifa_flags & (IFF_LOOPBACK)) ||
                !(ifa->ifa_flags & (IFF_RUNNING)))
            {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_PACKET)
            {
                struct sockaddr_ll *sl = (struct sockaddr_ll *)ifa->ifa_addr;
                std::ostringstream oss;
                oss << std::hex;
                for (int i = 0; i < sl->sll_halen; i++)
                {
                    oss << std::setfill('0') << std::setw(2) << (int)(sl->sll_addr[i]) << ((i + 1 != sl->sll_halen) ? "-" : "");
                }
                hw_addrs.push_back(oss.str());
            }
#elif defined(__APPLE__) && defined(__MACH__)
            if (strstr(ifa->ifa_name, "en") != ifa->ifa_name ||
                ifa->ifa_addr->sa_family != AF_LINK)
            {
                continue;
            }

            struct sockaddr_dl *sdl = (struct sockaddr_dl *)(ifa->ifa_addr);
            unsigned char *ptr = (unsigned char *)LLADDR(sdl);
            char buf[20];
            sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x", *ptr, *(ptr + 1), *(ptr + 2),
                    *(ptr + 3), *(ptr + 4), *(ptr + 5));
            hw_addrs.emplace_back(buf);
#endif
        }
        std::sort(hw_addrs.begin(), hw_addrs.end());
        freeifaddrs(ifaddr);
    }
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

HashTable *fetch_outmost_hashtable_from_ht(HashTable *ht, const char *arKey)
{
    zval **origin_zv;
    if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS &&
        Z_TYPE_PP(origin_zv) == IS_ARRAY)
    {
        return Z_ARRVAL_PP(origin_zv);
    }
    return nullptr;
}

bool fetch_outmost_long_from_ht(HashTable *ht, const char *arKey, long *result)
{
    zval **origin_zv;
    if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS &&
        Z_TYPE_PP(origin_zv) == IS_LONG)
    {
        *result = Z_LVAL_PP(origin_zv);
        return true;
    }
    return false;
}

zval *fetch_outmost_zval_from_ht(HashTable *ht, const char *arKey)
{
    zval **origin_zv;
    if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS)
    {
        return *origin_zv;
    }
    return nullptr;
}

bool fetch_source_in_ip_packets(char *local_ip, size_t len, char *url)
{
    struct hostent *server = nullptr;
    int backend_port = 0;
    php_url *resource = php_url_parse_ex(url, strlen(url));
    if (resource)
    {
        if (resource->host)
        {
            server = gethostbyname(resource->host);
        }
        if (resource->port)
        {
            backend_port = resource->port;
        }
        else
        {
            if (resource->scheme != NULL && strcmp(resource->scheme, "https") == 0)
            {
                backend_port = 443;
            }
            else
            {
                backend_port = 80;
            }
        }
        php_url_free(resource);
    }
    if (nullptr == server)
    {
        return false;
    }
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < 0)
    {
        return false;
    }
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    memcpy(&(serv.sin_addr.s_addr), server->h_addr, server->h_length);
    serv.sin_port = htons(backend_port);
    int err = connect(sock, (const struct sockaddr *)&serv, sizeof(serv));
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr *)&name, &namelen);
    const char *p = inet_ntop(AF_INET, &name.sin_addr, local_ip, len);
    if (nullptr == p)
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("inet_ntop error - error number : %d , error message : %s"), errno, strerror(errno));
        close(sock);
        return false;
    }
    close(sock);
    return true;
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

bool start_with(const std::string &str, const std::string &prefix)
{
    size_t len1 = str.length();
    size_t len2 = prefix.length();
    if (len1 < len2)
    {
        return false;
    }
    return (!str.compare(0, len2, prefix));
}

bool end_with(const std::string &str, const std::string &suffix)
{
    size_t len1 = str.length();
    size_t len2 = suffix.length();
    if (len1 < len2) {
        return false;
    }
    return (!str.compare(len1 - len2, len2, suffix));
}