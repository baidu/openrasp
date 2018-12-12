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
#include "JsonReader.h"
#include "utils/string.h"

namespace openrasp
{

JsonReader::JsonReader(const std::string &json_str)
{
  try
  {
    json::parse(json_str);
  }
  catch (json::parse_error &e)
  {
    has_error = true;
    std::ostringstream oss;
    oss << "message: " << e.what() << ';'
        << "exception id: " << e.id << ';'
        << "byte position of error: " << e.byte;
    error_msg = oss.str();
  }
}

const std::string JsonReader::_to_pointer(const std::vector<std::string> &keys)
{
  std::string pointer_str;
  for (std::string key : keys)
  {
    string_replace(key, "/", "~1"); //https://tools.ietf.org/html/rfc6901
    pointer_str.push_back('/');
    pointer_str.append(key);
  }
  return pointer_str;
}

std::string JsonReader::fetch_string(const std::vector<std::string> &keys, std::string &default_value)
{
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

int64_t JsonReader::fetch_int64(const std::vector<std::string> &keys, int64_t &default_value)
{
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

bool JsonReader::fetch_bool(const std::vector<std::string> &keys, bool &default_value)
{
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

void JsonReader::erase(const std::vector<std::string> &keys)
{
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  j.erase(ptr);
}

std::vector<std::string> JsonReader::fetch_object_keys(const std::vector<std::string> &keys)
{
  std::vector<std::string> result;
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  try
  {
    json::reference ref = j.at(ptr);
    for (json::iterator it = ref.begin(); it != ref.end(); ++it)
    {
      result.push_back(it.key());
    }
  }
  catch (...)
  {
  }
  return result;
}

std::vector<std::string> JsonReader::fetch_strings(const std::vector<std::string> &keys)
{
  std::vector<std::string> default_value;
  json::json_pointer ptr = json::json_pointer(_to_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

} // namespace openrasp
