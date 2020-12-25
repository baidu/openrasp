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

#ifndef _OPENRASP_CURL_RESPONSE_H_
#define _OPENRASP_CURL_RESPONSE_H_
#include "utils/json_reader.h"
#include <string>
#include <memory>
#include <curl/curl.h>
#include <functional>
#include "openrasp_hook.h"

namespace openrasp
{

class PluginUpdatePackage;

class BackendResponse
{

protected:
  long response_code = 0;
  std::string header_string;
  std::string response_string;

  bool parse_error = false;
  std::string error_msg;

  BaseReader *body_reader = nullptr;
  std::map<std::string, std::string> header_map;

public:
  static const int64_t default_int64;

  BackendResponse(){};
  BackendResponse(long response_code, std::string header_string, std::string response_string);
  virtual ~BackendResponse();

  virtual BaseReader *get_body_reader();
  virtual bool has_error() const;
  virtual bool http_code_ok() const;
  virtual long get_http_code() const;
  virtual std::string to_string() const;
  virtual std::string get_error_msg() const;
};

} // namespace openrasp
#endif