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
  std::string path;
  std::string query;
  bool parse_error = true;

public:
  Url();
  Url(const std::string &url_string);
  virtual void parse(const std::string &url_string);
  virtual bool has_error() const;
  virtual std::string get_scheme() const;
  virtual std::string get_host() const;
  virtual std::string get_port() const;
  virtual std::string get_path() const;
  virtual std::string get_query() const;
  virtual void set_scheme(const std::string &scheme);
  virtual void set_host(const std::string &host);
  virtual void set_port(const std::string &port);
  virtual void set_path(const std::string &path);
  virtual void set_query(const std::string &query);

  friend bool operator==(const openrasp::Url &lhs, const openrasp::Url &rhs);
};

} // namespace openrasp

#endif
