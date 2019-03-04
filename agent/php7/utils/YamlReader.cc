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
#include <fstream>

namespace openrasp
{
YamlReader::YamlReader()
{
}

YamlReader::YamlReader(const std::string &yaml_str)
{
  load(yaml_str);
}

void YamlReader::load(const std::string &content)
{
  try
  {
    node = YAML::Load(content);
  }
  catch (YAML::ParserException &e)
  {
    error = true;
    std::ostringstream oss;
    oss << "message: " << e.what();
    error_msg = oss.str();
  }
}

std::string YamlReader::fetch_string(const std::vector<std::string> &keys, const std::string &default_value)
{
  std::vector<Node> nodes;
  try
  {
    for (size_t i = 0; i < keys.size(); ++i)
    {
      if (0 == i)
      {
        nodes.push_back(node[keys[i]]);
      }
      else
      {
        nodes.push_back(nodes[i - 1][keys[i]]);
      }
    }
    return nodes[keys.size() - 1].as<std::string>();
  }
  catch (...)
  {
    return default_value;
  }
}
int64_t YamlReader::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value)
{
  std::vector<Node> nodes;
  try
  {
    for (size_t i = 0; i < keys.size(); ++i)
    {
      if (0 == i)
      {
        nodes.push_back(node[keys[i]]);
      }
      else
      {
        nodes.push_back(nodes[i - 1][keys[i]]);
      }
    }
    return nodes[keys.size() - 1].as<int64_t>();
  }
  catch (...)
  {
    return default_value;
  }
}
bool YamlReader::fetch_bool(const std::vector<std::string> &keys, const bool &default_value)
{
  std::vector<Node> nodes;
  try
  {
    for (size_t i = 0; i < keys.size(); ++i)
    {
      if (0 == i)
      {
        nodes.push_back(node[keys[i]]);
      }
      else
      {
        nodes.push_back(nodes[i - 1][keys[i]]);
      }
    }
    return nodes[keys.size() - 1].as<bool>();
  }
  catch (...)
  {
    return default_value;
  }
}
std::vector<std::string> YamlReader::fetch_object_keys(const std::vector<std::string> &keys)
{
  std::vector<std::string> result;
  std::vector<Node> nodes;
  try
  {
    for (size_t i = 0; i < keys.size(); ++i)
    {
      if (0 == i)
      {
        nodes.push_back(node[keys[i]]);
      }
      else
      {
        nodes.push_back(nodes[i - 1][keys[i]]);
      }
    }
    if (nodes[keys.size() - 1].IsMap())
    {
      for (YAML::const_iterator it = nodes[keys.size() - 1].begin(); it != nodes[keys.size() - 1].end(); ++it)
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
std::vector<std::string> YamlReader::fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value)
{
  std::vector<Node> nodes;
  try
  {
    std::vector<std::string> result;
    for (size_t i = 0; i < keys.size(); ++i)
    {
      if (0 == i)
      {
        nodes.push_back(node[keys[i]]);
      }
      else
      {
        nodes.push_back(nodes[i - 1][keys[i]]);
      }
    }

    if (nodes[keys.size() - 1].IsSequence())
    {
      for (YAML::const_iterator it = nodes[keys.size() - 1].begin(); it != nodes[keys.size() - 1].end(); ++it)
      {
        result.push_back(it->as<std::string>());
      }
    }
    return result;
  }
  catch (...)
  {
    return default_value;
  }
}

} // namespace openrasp
