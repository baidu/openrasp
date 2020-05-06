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
#include <cstdlib>
#include "json_reader.h"
#include "utils/json.h"
#include "openrasp_log.h"
#include "openrasp_error.h"

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

std::string JsonReader::fetch_string(const std::vector<std::string> &keys, const std::string &default_value,
                                     const std::function<std::string(const std::string &value)> &validator)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    std::string result;
    if (j.at(ptr).is_string())
    {
      result = j.at(ptr).get<std::string>();
    }
    else if (j.at(ptr).is_number())
    {
      result = std::to_string(j.at(ptr).get<int64_t>());
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be string");
    }
    if (nullptr != validator)
    {
      std::string error_description = validator(result);
      if (!error_description.empty())
      {
        throw nlohmann::detail::other_error::create(901, error_description);
      }
    }
    return result;
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value \"%s\""),
                     e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value.c_str());
    }
  }
  return default_value;
}

int64_t JsonReader::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value,
                                const std::function<std::string(int64_t value)> &validator)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    int64_t result = default_value;
    if (j.at(ptr).is_number())
    {
      result = j.at(ptr).get<int64_t>();
    }
    else if (j.at(ptr).is_string())
    {
      result = atoi(j.at(ptr).get<std::string>().c_str());
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be number");
    }
    if (nullptr != validator)
    {
      std::string error_description = validator(result);
      if (!error_description.empty())
      {
        throw nlohmann::detail::other_error::create(901, error_description);
      }
    }
    return result;
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value %" PRId64 " "),
                     e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value);
    }
  }
  return default_value;
}

bool JsonReader::fetch_bool(const std::vector<std::string> &keys, const bool &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    if (j.at(ptr).is_boolean())
    {
      return j.at(ptr);
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be boolean");
    }
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value \"%s\""),
                     e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value ? "true" : "false");
    }
  }
  return default_value;
}

std::vector<std::string> JsonReader::fetch_object_keys(const std::vector<std::string> &keys)
{
  std::vector<std::string> result;
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    json::reference ref = j.at(ptr);
    if (j.at(ptr).is_object())
    {
      for (json::iterator it = ref.begin(); it != ref.end(); ++it)
      {
        result.push_back(it.key());
      }
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be object");
    }
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\""),
                     e.what(), BaseReader::stringfy_keys(keys).c_str());
    }
  }
  return result;
}

std::vector<std::string> JsonReader::fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    if (j.at(ptr).is_array())
    {
      return j.at(ptr);
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be array");
    }
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value"),
                     e.what(), BaseReader::stringfy_keys(keys).c_str());
    }
  }
  return default_value;
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
  }
  return "";
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

size_t JsonReader::get_array_size(const std::vector<std::string> &keys)
{
  size_t result = 0;
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  try
  {
    json::reference ref = j.at(ptr);
    if (ref.is_array())
    {
      result = ref.size();
    }
    else
    {
      throw nlohmann::detail::other_error::create(900, "type should be array");
    }
  }
  catch (const nlohmann::detail::exception &e)
  {
    if (get_exception_report())
    {
      openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\""),
                     e.what(), BaseReader::stringfy_keys(keys).c_str());
    }
  }
  return result;
}

void JsonReader::write_map(const std::vector<std::string> &keys, const std::map<std::string, std::string> &value)
{
  json j_map(value);
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = j_map;
}

void JsonReader::write_vector(const std::vector<std::string> &keys, const std::vector<std::string> &value)
{
  json j_vec(value);
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = j_vec;
}

void JsonReader::write_int64_vector(const std::vector<std::string> &keys, const std::vector<int> &value)
{
  json j_vec(value);
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  j[ptr] = j_vec;
}

void JsonReader::update(const JsonReader &obj)
{
  j.update(obj.j);
}

} // namespace openrasp
