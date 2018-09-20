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

#ifndef _OPENRASP_CURL_HELPER_H_
#define _OPENRASP_CURL_HELPER_H_
#include <string>
#include <curl/curl.h>
#include "openrasp_hook.h"

namespace openrasp
{

struct ResponseInfo
{
    CURLcode res;
    std::string response_string;
    std::string header_string;
    long response_code;
    double elapsed;
};

void perform_curl(CURL *curl, const std::string url_string, const char *postdata, ResponseInfo &res_info);
} // namespace openrasp
#endif