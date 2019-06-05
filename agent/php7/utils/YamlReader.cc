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
#include <sstream>

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
        std::stringstream ss(content);
        YAML::Parser parser(ss);
        parser.GetNextDocument(doc);
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
    try
    {
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        std::string rst;
        *node >> rst;
        return rst;
    }
    catch (...)
    {
        return default_value;
    }
}
int64_t YamlReader::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value)
{
    try
    {
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        int64_t rst;
        *node >> rst;
        return rst;
    }
    catch (...)
    {
        return default_value;
    }
}
bool YamlReader::fetch_bool(const std::vector<std::string> &keys, const bool &default_value)
{
    try
    {
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        bool rst;
        *node >> rst;
        return rst;
    }
    catch (...)
    {
        return default_value;
    }
}
std::vector<std::string> YamlReader::fetch_object_keys(const std::vector<std::string> &keys)
{
    try
    {
        std::vector<std::string> rst;
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        if (node->Type() == YAML::NodeType::Map)
        {
            for (YAML::Iterator it = node->begin(); it != node->end(); ++it)
            {
                std::string key;
                it.first() >> key;
                rst.push_back(key);
            }
        }
        return rst;
    }
    catch (...)
    {
        return {};
    }
}
std::vector<std::string> YamlReader::fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value)
{
    try
    {
        std::vector<std::string> rst;
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        if (node->Type() == YAML::NodeType::Sequence)
        {
            for (YAML::Iterator it = node->begin(); it != node->end(); ++it)
            {
                std::string val;
                *it >> val;
                rst.push_back(val);
            }
        }
        return rst;
    }
    catch (...)
    {
        return default_value;
    }
}

} // namespace openrasp
