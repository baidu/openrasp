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

#include "openrasp.h"
#include "net.h"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "openrasp_log.h"

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

extern "C"
{
#include "ext/standard/url.h"
}

namespace openrasp
{

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
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL)
        {
            openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
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
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("getifaddrs() error: %s"), strerror(errno));
    }
    else
    {
        int n = 0;
        int s = 0;
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
                    openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("getifaddrs() error: getnameinfo failed - %s."), gai_strerror(s));
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
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("getifaddrs error: %s"), strerror(errno));
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

bool fetch_source_in_ip_packets(char *local_ip, size_t len, char *url)
{
    struct hostent *server = nullptr;
    int backend_port = 0;
    php_url *resource = php_url_parse_ex(url, strlen(url));
    if (resource)
    {
        if (resource->host)
        {
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3)
            server = gethostbyname(resource->host);
#else
            server = gethostbyname(resource->host->val);
#endif
        }
        if (resource->port)
        {
            backend_port = resource->port;
        }
        else
        {
            std::string scheme;
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3)
            scheme = std::string(resource->scheme);
#else
            scheme = std::string(resource->scheme->val, resource->scheme->len);
#endif
            if (scheme.find("https") == 0)
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
    if (nullptr != server)
    {
        struct sockaddr_in serv;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        fcntl(sock, F_SETFL, O_NONBLOCK);
        if (sock >= 0)
        {
            memset(&serv, 0, sizeof(serv));
            serv.sin_family = AF_INET;
            memcpy(&(serv.sin_addr.s_addr), server->h_addr, server->h_length);
            serv.sin_port = htons(backend_port);
            int err = connect(sock, (const struct sockaddr *)&serv, sizeof(serv));
            struct sockaddr_in name;
            socklen_t namelen = sizeof(name);
            err = getsockname(sock, (struct sockaddr *)&name, &namelen);
            const char *p = inet_ntop(AF_INET, &name.sin_addr, local_ip, len);
            close(sock);
            return nullptr != p;
        }
    }
    return false;
}

std::vector<std::string> lookup_host(const std::string &host)
{
    std::vector<std::string> ips;
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (errcode == 0)
    {
        while (res)
        {
            inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

            switch (res->ai_family)
            {
            case AF_INET:
                ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
                break;
            }
            inet_ntop(res->ai_family, ptr, addrstr, 100);
            ips.push_back(addrstr);
            res = res->ai_next;
        }
    }
    std::sort(ips.begin(), ips.end());
    return ips;
}

} // namespace openrasp
