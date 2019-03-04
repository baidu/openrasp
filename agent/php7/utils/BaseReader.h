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

#ifndef _OPENRASP_UTILS_BASE_READER_H_
#define _OPENRASP_UTILS_BASE_READER_H_

#include <string>
#include <vector>

namespace openrasp
{

class BaseReader
{
protected:
  bool error = false;
  std::string error_msg;

public:
  virtual std::string fetch_string(const std::vector<std::string> &keys, const std::string &default_value = "") = 0;
  virtual int64_t fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value = 0) = 0;
  virtual bool fetch_bool(const std::vector<std::string> &keys, const bool &default_value = false) = 0;
  virtual std::vector<std::string> fetch_object_keys(const std::vector<std::string> &keys) = 0;
  virtual std::vector<std::string> fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value = std::vector<std::string>()) = 0;
  virtual void load(const std::string &content) = 0;
  inline bool has_error() const
  {
    return error;
  }
  inline std::string get_error_msg() const
  {
    return error_msg;
  }
};

} // namespace openrasp

#endif
