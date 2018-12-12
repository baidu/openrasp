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

#include "YamlReader.h"

namespace openrasp
{

YamlReader::YamlReader(const std::string &yaml_str)
{
  node = YAML::Load(yaml_str);
}
std::string YamlReader::fetch_string(const std::vector<std::string> &keys, std::string &default_value)
{
  Node current;
  try
  {
    for (std::string key : keys)
    {
      current = current.IsDefined() ? current[key] : node[key];
    }
    return current.as<std::string>();
  }
  catch (...)
  {
    return default_value;
  }
}
int64_t YamlReader::fetch_int64(const std::vector<std::string> &keys, int64_t &default_value)
{
  Node current;
  try
  {
    for (std::string key : keys)
    {
      current = current.IsDefined() ? current[key] : node[key];
    }
    return current.as<int64_t>();
  }
  catch (...)
  {
    return default_value;
  }
}
bool YamlReader::fetch_bool(const std::vector<std::string> &keys, bool &default_value)
{
  Node current;
  try
  {
    for (std::string key : keys)
    {
      current = current.IsDefined() ? current[key] : node[key];
    }
    return current.as<bool>();
  }
  catch (...)
  {
    return default_value;
  }
}
void YamlReader::erase(const std::vector<std::string> &keys)
{
  Node current;
  try
  {
    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
      current = current.IsDefined() ? current[keys[i]] : node[keys[i]];
    }
    current.remove(keys[keys.size() - 1]);
  }
  catch (...)
  {
  }
}
std::vector<std::string> YamlReader::fetch_object_keys(const std::vector<std::string> &keys)
{
  std::vector<std::string> result;
  Node current;
  try
  {
    for (std::string key : keys)
    {
      current = current.IsDefined() ? current[key] : node[key];
    }
    if (current.IsMap())
    {
      for (YAML::const_iterator it = current.begin(); it != current.end(); ++it)
      {
        result.push_back(it->first.as<std::string>());
      }
    }
  }
  catch (...)
  {
  }
  return result;
}
std::vector<std::string> YamlReader::fetch_strings(const std::vector<std::string> &keys)
{
  std::vector<std::string> default_value;
  Node current;
  try
  {
    for (std::string key : keys)
    {
      current = current.IsDefined() ? current[key] : node[key];
    }
    if (current.IsSequence())
    {
      for (YAML::const_iterator it = current.begin(); it != current.end(); ++it)
      {
        default_value.push_back(it->as<std::string>());
      }
    }
  }
  catch (...)
  {
  }
  return default_value;
}

} // namespace openrasp
