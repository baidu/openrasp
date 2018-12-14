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

#include <sstream>
#include "JsonWriter.h"
#include "utils/json.h"

namespace openrasp
{

JsonWriter::JsonWriter()
{
}
void JsonWriter::write_bool(const std::vector<std::string> &keys, const bool &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = value;
}
void JsonWriter::write_int64(const std::vector<std::string> &keys, const int64_t &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = value;
}
void JsonWriter::write_string(const std::vector<std::string> &keys, const std::string &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = value;
}
std::string JsonWriter::dump(bool pretty)
{
  try
  {
    return j.dump(pretty ? 4 : -1);
  }
  catch (...)
  {
    return "";
  }
}

} // namespace openrasp
