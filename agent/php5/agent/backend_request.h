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
  const std::string url;
  const char *post_data;

public:
  BackendRequest(const std::string &url, const char *post_data);
  virtual ~BackendRequest();
  std::shared_ptr<BackendResponse> curl_perform();
  CURLcode get_curl_code() const;
  const char *get_curl_err_msg() const;
};

} // namespace openrasp
#endif