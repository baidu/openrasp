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

#ifndef _OPENRASP_CURL_RESPONSE_H_
#define _OPENRASP_CURL_RESPONSE_H_
#include <string>
#include <memory>
#include <curl/curl.h>
#include <functional>
#include "plugin_update_pkg.h"
#include "openrasp_hook.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/pointer.h"
#include "third_party/rapidjson/writer.h"

namespace openrasp
{

class PluginUpdatePackage;

class BackendResponse
{
public:
  bool parse_error = false;
  long response_code = 0;
  std::string header_string;
  std::string response_string;
  rapidjson::Document document;

public:
  BackendResponse(){};
  BackendResponse(long response_code, std::string header_string, std::string response_string);

  bool has_error() const;
  bool http_code_ok() const;
  long get_http_code() const;

  bool fetch_status(int64_t &status);
  bool fetch_description(std::string &description);

  bool erase_value(const char *key);
  bool fetch_int64(const char *key, int64_t &target);
  bool fetch_string(const char *key, std::string &target);
  bool stringify_object(const char *key, std::string &target);

  std::shared_ptr<PluginUpdatePackage> build_plugin_update_package();
};

std::shared_ptr<BackendResponse> curl_perform(CURL *curl, const std::string &url_string, const char *postdata);

} // namespace openrasp
#endif