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

#include "backend_response.h"
#include "openrasp_ini.h"
#include "utils/digest.h"
#include "utils/string.h"

namespace openrasp
{
const int64_t BackendResponse::default_int64 = -999;

BackendResponse::BackendResponse(long response_code, std::string header_string, std::string response_string)
{
    this->response_code = response_code;
    this->header_string = header_string;
    this->response_string = response_string;
    json_reader.load(response_string.c_str());
    this->parse_error = json_reader.has_error();
    this->error_msg = json_reader.get_error_msg();
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

int64_t BackendResponse::fetch_status()
{
    return json_reader.fetch_int64({"status"}, BackendResponse::default_int64);
}

std::string BackendResponse::fetch_description()
{
    return json_reader.fetch_string({"description"}, "");
}

std::shared_ptr<PluginUpdatePackage> BackendResponse::build_plugin_update_package(const std::string &local_md5)
{
    std::shared_ptr<PluginUpdatePackage> result = nullptr;
    std::string plugin = fetch_string({"data", "plugin", "plugin"}, "");
    std::string md5 = fetch_string({"data", "plugin", "md5"}, "");
    if (!plugin.empty() && !md5.empty() && md5 != local_md5)
    {
        std::string cal_md5 =
            md5sum(static_cast<const void *>(plugin.c_str()), plugin.length());
        if (cal_md5 != md5)
        {
            return nullptr;
        }
        std::string version = fetch_string({"data", "plugin", "version"}, "");
        if (version.empty())
        {
            return nullptr;
        }
        result = make_shared<PluginUpdatePackage>(plugin, version, md5);
    }
    return result;
}

int64_t BackendResponse::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value)
{
    return json_reader.fetch_int64(keys, default_value);
}

std::string BackendResponse::fetch_string(const std::vector<std::string> &keys, const std::string &default_value)
{
    return json_reader.fetch_string(keys, default_value);
}

std::string BackendResponse::stringify_object(const std::vector<std::string> &keys, bool pretty)
{
    return json_reader.dump(keys, pretty);
}

void BackendResponse::erase_value(const std::vector<std::string> &keys)
{
    json_reader.erase(keys);
}

bool BackendResponse::verify(openrasp_error_code error_code)
{
    if (has_error())
    {
        openrasp_error(LEVEL_WARNING, error_code, _("Fail to parse response body, error message %s."), error_msg.c_str());
        return false;
    }
    if (!http_code_ok())
    {
        openrasp_error(LEVEL_WARNING, error_code, _("Unexpected http response code: %ld."),
                       get_http_code());
        return false;
    }

    int64_t status = fetch_status();
    std::string description = fetch_description();
    if (0 != status)
    {
        openrasp_error(LEVEL_WARNING, error_code, _("API error: %ld, description: %s"),
                       status, description.c_str());
        return false;
    }
    return true;
}

std::vector<std::string> BackendResponse::fetch_object_keys(const std::vector<std::string> &keys)
{
    return json_reader.fetch_object_keys(keys);
}

std::vector<std::string> BackendResponse::fetch_string_array(const std::vector<std::string> &keys)
{
    return json_reader.fetch_strings(keys, {});
}

std::map<std::string, std::vector<std::string>> BackendResponse::build_hook_white_map(const std::vector<std::string> &keys)
{
    std::map<std::string, std::vector<std::string>> hook_white_map;
    std::vector<std::string> url_keys = fetch_object_keys(keys);
    std::vector<std::string> tmp_keys(keys);
    for (auto &key_item : url_keys)
    {
        tmp_keys.push_back(key_item);
        std::vector<std::string> white_types = fetch_string_array(tmp_keys);
        tmp_keys.pop_back();
        hook_white_map.insert({key_item, white_types});
    }
    return hook_white_map;
}

std::string BackendResponse::to_string() const
{
    return "Response_code: " + std::to_string(response_code) + "\nheader_string: " + header_string + "body:" + response_string;
}

} // namespace openrasp