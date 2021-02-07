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

#include "request.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "openrasp_content_type.h"

namespace openrasp
{
namespace request
{
Request::Request(/* args */)
{
}

Request::~Request()
{
}

void Request::update_id()
{
    auto time_point = std::chrono::steady_clock::now();
    long long nano = time_point.time_since_epoch().count();
    std::size_t hash = std::hash<std::string>{}(std::to_string(nano));
    std::ostringstream oss;
    oss << std::hex;
    oss << std::setfill('0') << std::setw(16) << hash;
    oss << std::setfill('0') << std::setw(16) << nano;
    this->id = oss.str();
}

std::string Request::get_id() const
{
    return id;
}

void Request::set_method(const std::string &method)
{
    std::string lower_method = method;
    std::transform(lower_method.begin(), lower_method.end(), lower_method.begin(), ::tolower);
    this->method = lower_method;
}

std::string Request::get_method() const
{
    return method;
}

void Request::set_remote_addr(const std::string &remote_addr)
{
    this->remote_addr = remote_addr;
}

std::string Request::get_remote_addr() const
{
    return remote_addr;
}

void Request::set_document_root(const std::string &document_root)
{
    this->document_root = document_root;
}

std::string Request::get_document_root() const
{
    return document_root;
}

void Request::set_header(const std::map<std::string, std::string> &header)
{
    this->header = header;
}

std::map<std::string, std::string> Request::get_header() const
{
    return header;
}

std::string Request::get_header(const std::string &key) const
{
    auto found = header.find(key);
    if (found != header.end())
    {
        return found->second;
    }
    return "";
}

void Request::set_body_length(size_t body_len)
{
    this->body_len = body_len;
}

Parameter &Request::get_parameter()
{
    if (!parameter.get_initialized())
    {
        std::string content_type = get_header("content-type");
        OpenRASPContentType::ContentType k_type = OpenRASPContentType::classify_content_type(content_type);
        switch (k_type)
        {
        case OpenRASPContentType::ContentType::cApplicationJson:
            parameter.update_json_str();
            break;
        case OpenRASPContentType::ContentType::cApplicationForm:
            parameter.update_form_str();
            break;
        case OpenRASPContentType::ContentType::cMultipartForm:
            parameter.update_form_str();
            parameter.update_multipart_files();
            break;
        default:
            parameter.update_body_str(body_len);
            break;
        }
        parameter.set_initialized(true);
    }
    return parameter;
}

void Request::clear()
{
    body_len = 0;
    id.clear();
    method.clear();
    remote_addr.clear();
    document_root.clear();
    header.clear();
    url.clear();
    parameter.clear();
}
} // namespace request

} // namespace openrasp