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

#include "backend_response.h"
#include "openrasp_ini.h"
#include "utils/digest.h"

namespace openrasp
{

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

std::shared_ptr<BackendResponse> curl_perform(CURL *curl, const std::string &url_string, const char *postdata)
{
    if (curl)
    {
        CURLcode res = CURL_LAST;
        long response_code;
        std::string header_string;
        std::string response_string;
        struct curl_slist *chunk = nullptr;
        std::string auth_header = "X-OpenRASP-AppID: " + std::string(openrasp_ini.app_id);
        chunk = curl_slist_append(chunk, auth_header.c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        if (postdata)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response_string));
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(header_string));
        res = curl_easy_perform(curl);
        if (CURLE_OK != res)
        {
            openrasp_error(E_WARNING, AGENT_ERROR, _("CURL error code: %d while access %s."), res, url_string.c_str());
        }
        else
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(response_code));
            return make_shared<BackendResponse>(response_code, header_string, response_string);
        }
    }
    return nullptr;
}

BackendResponse::BackendResponse(long response_code, std::string header_string, std::string response_string)
{
    this->response_code = response_code;
    this->header_string = header_string;
    this->response_string = response_string;
    document.Parse(response_string.c_str());
    this->parse_error = document.HasParseError();
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

bool BackendResponse::fetch_status(int64_t &status)
{
    return fetch_int64("/status", status);
}

bool BackendResponse::fetch_description(std::string &description)
{
    return fetch_string("/description", description);
}

std::shared_ptr<PluginUpdatePackage> BackendResponse::build_plugin_update_package()
{
    std::string plugin;
    std::string md5;
    std::string version;
    std::shared_ptr<PluginUpdatePackage> result = nullptr;
    if (fetch_string("/data/plugin/plugin", plugin) &&
        fetch_string("/data/plugin/md5", md5))
    {
        std::string cal_md5 =
            md5sum(static_cast<const void *>(plugin.c_str()), plugin.length());
        if (cal_md5 != md5)
        {
            return nullptr;
        }
        bool has_version = fetch_string("/data/plugin/version", version);
        if (!has_version)
        {
            return nullptr;
        }
        result = make_shared<PluginUpdatePackage>(plugin, version, md5);
        std::string algorithm;
        if (stringify_object("/data/config/algorithm.config", algorithm))
        {
            result->set_algorithm(algorithm);
        }
        erase_value("/data/config/algorithm.config");
    }
    return result;
}

bool BackendResponse::fetch_int64(const char *key, int64_t &target)
{
    const rapidjson::Pointer pointer(key);
    const rapidjson::Value *value = rapidjson::GetValueByPointer(document, pointer);
    if (value && value->IsInt64())
    {
        target = value->GetInt64();
        return true;
    }
    return false;
}

bool BackendResponse::fetch_string(const char *key, std::string &target)
{
    const rapidjson::Pointer pointer(key);
    const rapidjson::Value *value = rapidjson::GetValueByPointer(document, pointer);
    if (value && value->IsString())
    {
        target = value->GetString();
        return true;
    }
    return false;
}

bool BackendResponse::stringify_object(const char *key, std::string &target)
{
    const rapidjson::Pointer pointer(key);
    const rapidjson::Value *value = rapidjson::GetValueByPointer(document, pointer);
    if (value && value->IsObject())
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        value->Accept(writer);
        target = sb.GetString();
        return true;
    }
    return true;
}

bool BackendResponse::erase_value(const char *key)
{
    const rapidjson::Pointer pointer(key);
    return rapidjson::EraseValueByPointer(document, pointer);
}

} // namespace openrasp