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

#include "yaml_reader.h"
#include <sstream>
#include "openrasp_log.h"
#include "openrasp_error.h"

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

std::string YamlReader::fetch_string(const std::vector<std::string> &keys, const std::string &default_value,
                                     const std::function<std::string(const std::string &value)> &validator)
{
    try
    {
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        if (node->Type() == YAML::NodeType::Scalar)
        {
            std::string rst;
            *node >> rst;
            if (nullptr != validator)
            {
                std::string error_description = validator(rst);
                if (!error_description.empty())
                {
                    throw YAML::RepresentationException(node->GetMark(), error_description);
                }
            }
            return rst;
        }
        else
        {
            throw YAML::RepresentationException(node->GetMark(), "type should be string");
        }
    }
    catch (const YAML::KeyNotFound &e)
    {
    }
    catch (const YAML::BadDereference &e)
    {
    }
    catch (const YAML::Exception &e)
    {
        if (get_exception_report())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value \"%s\""),
                           e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value.c_str());
        }
    }
    return default_value;
}
int64_t YamlReader::fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value,
                                const std::function<std::string(int64_t value)> &validator)
{
    try
    {
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        if (node->Type() == YAML::NodeType::Scalar)
        {
            int64_t rst;
            *node >> rst;

            if (nullptr != validator)
            {
                std::string error_description = validator(rst);
                if (!error_description.empty())
                {
                    throw YAML::RepresentationException(node->GetMark(), error_description);
                }
            }
            return rst;
        }
        else
        {
            throw YAML::RepresentationException(node->GetMark(), "type should be number");
        }
    }
    catch (const YAML::KeyNotFound &e)
    {
    }
    catch (const YAML::BadDereference &e)
    {
    }
    catch (const YAML::Exception &e)
    {
        if (get_exception_report())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value %" PRId64 " "),
                           e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value);
        }
    }
    return default_value;
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
        if (node->Type() == YAML::NodeType::Scalar)
        {
            bool rst = false;
            *node >> rst;
            return rst;
        }
        else
        {
            throw YAML::RepresentationException(node->GetMark(), "type should be boolen");
        }
    }
    catch (const YAML::KeyNotFound &e)
    {
    }
    catch (const YAML::BadDereference &e)
    {
    }
    catch (const YAML::Exception &e)
    {
        if (get_exception_report())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value %s"),
                           e.what(), BaseReader::stringfy_keys(keys).c_str(), default_value ? "true" : "false");
        }
    }
    return default_value;
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
        else if (node->Type() == YAML::NodeType::Null)
        {
            //skip
        }
        else
        {
            throw YAML::RepresentationException(node->GetMark(), "type should be map");
        }
        return rst;
    }
    catch (const YAML::KeyNotFound &e)
    {
    }
    catch (const YAML::BadDereference &e)
    {
    }
    catch (const YAML::Exception &e)
    {
        if (get_exception_report())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\""),
                           e.what(), BaseReader::stringfy_keys(keys).c_str());
        }
    }
    return {};
}
std::vector<std::string> YamlReader::fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value)
{
    try
    {
        std::vector<std::string> rst = default_value;
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        if (node->Type() == YAML::NodeType::Sequence)
        {
            rst.clear();
            for (YAML::Iterator it = node->begin(); it != node->end(); ++it)
            {
                std::string val;
                *it >> val;
                rst.push_back(val);
            }
        }
        else
        {
            throw YAML::RepresentationException(node->GetMark(), "type should be sequence");
        }
        return rst;
    }
    catch (const YAML::KeyNotFound &e)
    {
    }
    catch (const YAML::BadDereference &e)
    {
    }
    catch (const YAML::Exception &e)
    {
        if (get_exception_report())
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("%s, config \"%s\" use the default value"),
                           e.what(), BaseReader::stringfy_keys(keys).c_str());
        }
    }
    return default_value;
}

std::string YamlReader::dump(const std::vector<std::string> &keys, bool pretty)
{
    std::string result;
    try
    {
        std::vector<std::string> rst;
        const YAML::Node *node = &doc;
        for (auto key : keys)
        {
            node = &(*node)[key];
        }
        YAML::Emitter emitter;
        emitter << *node;
        result = std::string(emitter.c_str());
        return result;
    }
    catch (...)
    {
    }
    return result;
}

std::string YamlReader::dump(bool pretty)
{
    return dump({}, pretty);
}

std::string YamlReader::detect_unknown_config_key()
{
    static const std::set<std::string> php_known_keys = {
        "plugin.timeout.millis",
        "plugin.maxstack",
        "plugin.filter",
        "log.maxburst",
        "log.maxstack",
        "log.maxbackup",
        "syslog.enable",
        "syslog.tag",
        "syslog.url",
        "syslog.facility",
        "syslog.connection_timeout",
        "syslog.read_timeout",
        "syslog.reconnect_interval",
        "block.status_code",
        "block.redirect_url",
        "block.content_json",
        "block.content_xml",
        "block.content_html",
        "inject.urlprefix",
        "inject.custom_headers",
        "body.maxbytes",
        "clientip.header",
        "security.weak_passwords",
        "lru.max_size",
        "debug.level",
        "hook.white",
        "response.sampler_interval",
        "response.sampler_burst",
        "decompile.enable"};
    std::vector<std::string> found_keys = fetch_object_keys({});
    for (auto &key : found_keys)
    {
        auto found = php_known_keys.find(key);
        if (found == php_known_keys.end())
        {
            return key;
        }
    }
    return "";
}

} // namespace openrasp
