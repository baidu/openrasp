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
#include <map>
#include "url.h"
#include "parameter.h"

namespace openrasp
{
namespace request
{
class Request
{
private:
    /* data */
    std::string id;
    std::string method;
    std::string remote_addr;
    std::string document_root;
    std::map<std::string, std::string> header;
    size_t body_len;

    Parameter parameter;

public:
    Url url;

    Request(/* args */);
    virtual ~Request();
    void update_id();
    std::string get_id() const;
    void set_method(const std::string &method);
    std::string get_method() const;
    void set_remote_addr(const std::string &remote_addr);
    std::string get_remote_addr() const;
    void set_document_root(const std::string &document_root);
    std::string get_document_root() const;
    void set_header(const std::map<std::string, std::string> &header);
    std::map<std::string, std::string> get_header() const;
    std::string get_header(const std::string &key) const;
    void set_body_length(size_t body_len);

    Parameter &get_parameter();

    void clear();
};
} // namespace request

} // namespace openrasp