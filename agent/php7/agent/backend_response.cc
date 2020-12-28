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

#include "backend_response.h"
#include "openrasp_ini.h"
#include "utils/digest.h"
#include "utils/string.h"
#include "openrasp_content_type.h"
#include <sstream>

namespace openrasp
{
const int64_t BackendResponse::default_int64 = -999;

BackendResponse::BackendResponse(long response_code, std::string header_string, std::string response_string)
{
    this->response_code = response_code;
    this->header_string = header_string;
    this->response_string = response_string;

    {
        std::istringstream iss(header_string);
        std::string line;
        while (std::getline(iss, line))
        {
            std::size_t found = line.find(':');
            if (found != std::string::npos && found + 1 < line.length())
            {
                std::string key = line.substr(0, found);
                for (auto &ch : key)
                {
                    ch = std::tolower(ch);
                }
                std::string value = line.substr(found + 1);
                header_map[key] = value;
            }
        }
    }

    auto found = header_map.find("content-type");
    if (found != header_map.end())
    {
        if (OpenRASPContentType::classify_content_type(found->second) == OpenRASPContentType::ContentType::cApplicationJson)
        {
            body_reader = new JsonReader();
            body_reader->load(response_string.c_str());
            parse_error = body_reader->has_error();
            error_msg = body_reader->get_error_msg();
            return;
        }
    }
    this->error_msg = "Only application/json supported.";
    this->parse_error = true;
}

BackendResponse::~BackendResponse()
{
    if (nullptr != body_reader)
    {
        delete body_reader;
    }
}

bool BackendResponse::has_error() const
{
    return parse_error;
}

long BackendResponse::get_http_code() const
{
    return response_code;
}

bool BackendResponse::http_code_ok() const
{
    return (response_code >= 200 && response_code < 300);
}

std::string BackendResponse::to_string() const
{
    return "Response_code: " + std::to_string(response_code) + "\nheader_string: " + header_string + "body:" + response_string;
}

BaseReader *BackendResponse::get_body_reader()
{
    return body_reader;
}

std::string BackendResponse::get_error_msg() const
{
    return error_msg;
}

} // namespace openrasp