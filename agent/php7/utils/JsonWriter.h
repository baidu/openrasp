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

#ifndef _OPENRASP_UTILS_JSON_WRITER_H_
#define _OPENRASP_UTILS_JSON_WRITER_H_

#ifdef snprintf
#undef snprintf
#endif
#define snprintf snprintf
//php define snprintf ...

#include "third_party/json/json.hpp"

#ifdef snprintf
#undef snprintf
#endif
#define snprintf ap_php_snprintf

namespace openrasp
{

using json = nlohmann::json;

class JsonWriter
{
private:
  json j;

public:
  JsonWriter();
  void write_bool(const std::vector<std::string> &keys, const bool &value);
  void write_int64(const std::vector<std::string> &keys, const int64_t &value);
  void write_string(const std::vector<std::string> &keys, const std::string &value);
  std::string dump(bool pretty = false);
};

} // namespace openrasp

#endif
