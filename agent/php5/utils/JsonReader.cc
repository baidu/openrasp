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
#include "utils/json.h"

namespace openrasp
{

JsonReader::JsonReader()
{
}

JsonReader::JsonReader(const std::string &json_str)
{
  load(json_str);
}

void JsonReader::load(const std::string &content)
{
  try
  {
    j = json::parse(content);
  }
  catch (json::parse_error &e)
  {
    error = true;
    std::ostringstream oss;
    oss << "message: " << e.what() << ';'
        << "exception id: " << e.id << ';'
        << "byte position of error: " << e.byte;
    error_msg = oss.str();
  }
}

std::string JsonReader::fetch_string(const std::vector<std::string> &keys, const std::string &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

int64_t JsonReader::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

bool JsonReader::fetch_bool(const std::vector<std::string> &keys, const bool &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
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
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j.erase(ptr);
}

std::vector<std::string> JsonReader::fetch_object_keys(const std::vector<std::string> &keys)
{
  std::vector<std::string> result;
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
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

std::vector<std::string> JsonReader::fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    return j.at(ptr);
  }
  catch (...)
  {
    return default_value;
  }
}

std::string JsonReader::dump(const std::vector<std::string> &keys, bool pretty)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    json::reference ref = j.at(ptr);
    return ref.dump(pretty ? 4 : -1);
  }
  catch (...)
  {
    return "";
  }
}

std::string JsonReader::dump(bool pretty)
{
  return dump({}, pretty);
}

void JsonReader::write_int64(const std::vector<std::string> &keys, const int64_t &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = value;
}
void JsonReader::write_string(const std::vector<std::string> &keys, const std::string &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = value;
}

void JsonReader::write_map_to_array(const std::vector<std::string> &keys, const std::string fkey, const std::string skey,
                                    const std::map<std::string, std::string> &value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  json array = json::array();
  for (auto iter : value)
  {
    json item;
    item[fkey] = iter.first;
    item[skey] = iter.second;
    array.push_back(item);
  }
  j[ptr] = array;
}

} // namespace openrasp
