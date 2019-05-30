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

#ifndef _OPENRASP_CURL_RESPONSE_H_
#define _OPENRASP_CURL_RESPONSE_H_
#include "utils/JsonReader.h"
#include <string>
#include <memory>
#include <curl/curl.h>
#include <functional>
#include "plugin_update_pkg.h"
#include "openrasp_hook.h"

namespace openrasp
{

class PluginUpdatePackage;

class BackendResponse
{

public:
  static const int64_t default_int64;
  bool parse_error = false;
  long response_code = 0;
  std::string header_string;
  std::string response_string;
  std::string error_msg;
  JsonReader json_reader;

public:
  BackendResponse(){};
  BackendResponse(long response_code, std::string header_string, std::string response_string);

  bool has_error() const;
  bool http_code_ok() const;
  long get_http_code() const;
  std::string to_string() const;

  int64_t fetch_status();
  std::string fetch_description();

  bool verify(openrasp_error_code error_code);

  int64_t fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value = BackendResponse::default_int64);
  std::string fetch_string(const std::vector<std::string> &keys, const std::string &default_value = "");
  std::string stringify_object(const std::vector<std::string> &keys, bool pretty = false);
  void erase_value(const std::vector<std::string> &keys);

  std::shared_ptr<PluginUpdatePackage> build_plugin_update_package(const std::string &local_md5);
  std::vector<std::string> fetch_object_keys(const std::vector<std::string> &keys);
  std::vector<std::string> fetch_string_array(const std::vector<std::string> &keys);
  std::map<std::string, std::vector<std::string>> build_hook_white_map(const std::vector<std::string> &keys);
};

} // namespace openrasp
#endif