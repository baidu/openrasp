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

#ifndef _OPENRASP_CURL_REQUEST_H_
#define _OPENRASP_CURL_REQUEST_H_
#include <string>
#include <memory>
#include <curl/curl.h>
#include <functional>
#include "backend_response.h"
#include "openrasp_hook.h"

namespace openrasp
{

class BackendResponse;

class BackendRequest
{

private:
  CURL *curl = nullptr;
  CURLcode curl_code;
  struct curl_slist *chunk = nullptr;

  void add_header(const std::string &header);
  void set_custom_headers();

public:
  BackendRequest();
  virtual ~BackendRequest();
  std::shared_ptr<BackendResponse> curl_perform();
  CURLcode get_curl_code() const;
  const char *get_curl_err_msg() const;
  void set_url(const std::string &url);
  void add_post_fields(const std::string &post_data);
};

} // namespace openrasp
#endif