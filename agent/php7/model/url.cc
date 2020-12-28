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

#include "url.h"

namespace openrasp
{
namespace request
{
Url::Url()
{
}

Url::~Url()
{
}

void Url::set_request_scheme(const std::string &request_scheme)
{
    this->request_scheme = !request_scheme.empty() ? request_scheme : "http";
}

void Url::set_http_host(const std::string &http_host)
{
    this->http_host = http_host;
}

void Url::set_server_name(const std::string &server_name)
{
    this->server_name = server_name;
}

void Url::set_server_addr(const std::string &server_addr)
{
    this->server_addr = server_addr;
}

void Url::set_request_uri(const std::string &request_uri)
{
    this->request_uri = request_uri;
}

void Url::set_query_string(const std::string &query_string)
{
    this->query_string = query_string;
}

void Url::set_port(const std::string &server_port)
{
    if (!server_port.empty())
    {
        this->port = std::stoi(server_port);
    }
    else
    {
        this->port = 80;
    }
}

void Url::clear()
{
    request_scheme.clear();
    http_host.clear();
    server_name.clear();
    server_addr.clear();
    request_uri.clear();
    query_string.clear();
    port = 0;
}

std::string Url::get_request_scheme() const
{
    return request_scheme;
}

std::string Url::get_http_host() const
{
    return http_host;
}

std::string Url::get_server_name() const
{
    return server_name;
}

std::string Url::get_server_addr() const
{
    return server_addr;
}

std::string Url::get_request_uri() const
{
    return request_uri;
}

std::string Url::get_query_string() const
{
    return query_string;
}

int Url::get_port() const
{
    return port;
}

std::string Url::get_complete_url() const
{
    std::string complete_url;
    complete_url.append(request_scheme).append("://");
    if (!http_host.empty())
    {
        complete_url.append(http_host);
    }
    else
    {
        complete_url.append((!server_name.empty() ? server_name : server_addr));
        if (80 != port)
        {
            complete_url.append(":").append(std::to_string(port));
        }
    }
    complete_url.append(request_uri);
    return complete_url;
}

std::string Url::get_real_host() const
{
    std::string real_host;
    if (!http_host.empty())
    {
        real_host.append(http_host);
    }
    else
    {
        real_host.append((!server_name.empty() ? server_name : server_addr));
        if (80 != port)
        {
            real_host.append(":").append(std::to_string(port));
        }
    }
    return real_host;
}

std::string Url::get_path() const
{
    auto found = request_uri.find("?");
    if (found != std::string::npos)
    {
        return request_uri.substr(0, found);
    }
    else
    {
        return request_uri;
    }
}
} // namespace request

} // namespace openrasp
