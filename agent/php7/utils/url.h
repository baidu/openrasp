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

#ifndef _OPENRASP_UTILS_URL_H_
#define _OPENRASP_UTILS_URL_H_

#include <string>

namespace openrasp
{

class Url
{
  private:
    std::string scheme;
    std::string host;
    std::string port;
    bool parse_error = false;

  public:
    Url(std::string url_string);
    bool has_error() const;
    std::string get_host() const;
    std::string get_port() const;
};

} // namespace openrasp

#endif
