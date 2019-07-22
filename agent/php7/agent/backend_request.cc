/*
 * Copyright 2017-2019 Baidu Inc.
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

#include "backend_request.h"
#include "openrasp_ini.h"
#include "openrasp_log.h"

namespace openrasp
{

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

BackendRequest::BackendRequest(const std::string &url, const char *post_data)
    : url(url), post_data(post_data)
{
    curl = curl_easy_init();
    curl_code = CURL_LAST;
}

BackendRequest::~BackendRequest()
{
    if (nullptr != curl)
    {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

std::shared_ptr<BackendResponse> BackendRequest::curl_perform()
{
    if (curl)
    {
        long response_code;
        std::string header_string;
        std::string response_string;
        struct curl_slist *chunk = nullptr;
        std::string auth_header_id = "X-OpenRASP-AppID: " + std::string(openrasp_ini.app_id);
        chunk = curl_slist_append(chunk, auth_header_id.c_str());
        std::string auth_header_secret = "X-OpenRASP-AppSecret: " + std::string(openrasp_ini.app_secret);
        chunk = curl_slist_append(chunk, auth_header_secret.c_str());
        curl_code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        if (post_data)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response_string));
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(header_string));
#if LIBCURL_VERSION_NUM < 0x071506
        curl_easy_setopt(curl, CURLOPT_ENCODING, "");
#else
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
#endif
        curl_code = curl_easy_perform(curl);
        if (CURLE_OK == curl_code)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(response_code));
            return make_shared<BackendResponse>(response_code, header_string, response_string);
        }
    }
    return nullptr;
}

CURLcode BackendRequest::get_curl_code() const
{
    return curl_code;
}

const char *BackendRequest::get_curl_err_msg() const
{
    return curl_easy_strerror(curl_code);
}

} // namespace openrasp