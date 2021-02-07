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

#pragma once

#include <string>

namespace openrasp
{
namespace request
{
class Url
{
private:
    /* data */
    std::string request_scheme;
    std::string http_host;
    std::string server_name;
    std::string server_addr;
    std::string request_uri;
    std::string query_string;
    int port;

public:
    Url(/* args */);
    virtual ~Url();

    void set_request_scheme(const std::string &request_scheme);
    void set_http_host(const std::string &http_host);
    void set_server_name(const std::string &server_name);
    void set_server_addr(const std::string &server_addr);
    void set_request_uri(const std::string &request_uri);
    void set_query_string(const std::string &query_string);
    void set_port(const std::string &server_port);
    void clear();

    std::string get_request_scheme() const;
    std::string get_http_host() const;
    std::string get_server_name() const;
    std::string get_server_addr() const;
    std::string get_request_uri() const;
    std::string get_query_string() const;
    std::string get_complete_url() const;
    std::string get_path() const;
    std::string get_real_host() const;
    int get_port() const;
};
} // namespace request

} // namespace openrasp
