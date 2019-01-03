/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "third_party/rapidjson/error/en.h"
#include "openrasp_config.h"

namespace openrasp
{
OpenraspConfig::OpenraspConfig(string &config, FromType type)
    : OpenraspConfig()
{
    From(config, type);
}
template <>
string OpenraspConfig::GetFromJson(const string &key, const string &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsString())
    {
        return string(it->value.GetString(), it->value.GetStringLength());
    }
    else
    {
        return default_value;
    }
}
template <>
int64_t OpenraspConfig::GetFromJson(const string &key, const int64_t &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsInt64())
    {
        return it->value.GetInt64();
    }
    else
    {
        return default_value;
    }
}
template <>
double OpenraspConfig::GetFromJson(const string &key, const double &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsDouble())
    {
        return it->value.GetDouble();
    }
    else
    {
        return default_value;
    }
}
template <>
bool OpenraspConfig::GetFromJson(const string &key, const bool &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsBool())
    {
        return it->value.GetBool();
    }
    else
    {
        return default_value;
    }
}
template <>
vector<string> OpenraspConfig::GetArrayFromJson(const string &key, const vector<string> &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsArray())
    {
        vector<string> arr;
        for (auto &item : it->value.GetArray())
        {
            if (!item.IsString())
            {
                return default_value;
            }
            arr.emplace_back(item.GetString(), item.GetStringLength());
        }
        return arr;
    }
    else
    {
        return default_value;
    }
}
template <>
vector<int64_t> OpenraspConfig::GetArrayFromJson(const string &key, const vector<int64_t> &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsArray())
    {
        vector<int64_t> arr;
        for (auto &item : it->value.GetArray())
        {
            if (!item.IsInt64())
            {
                return default_value;
            }
            arr.emplace_back(item.GetInt64());
        }
        return arr;
    }
    else
    {
        return default_value;
    }
}
template <>
vector<double> OpenraspConfig::GetArrayFromJson(const string &key, const vector<double> &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsArray())
    {
        vector<double> arr;
        for (auto &item : it->value.GetArray())
        {
            if (!item.IsDouble())
            {
                return default_value;
            }
            arr.emplace_back(item.GetDouble());
        }
        return arr;
    }
    else
    {
        return default_value;
    }
}
template <>
vector<bool> OpenraspConfig::GetArrayFromJson(const string &key, const vector<bool> &default_value) const
{
    auto it = jsonObj->FindMember(key.c_str());
    if (it != jsonObj->MemberEnd() &&
        it->value.IsArray())
    {
        vector<bool> arr;
        for (auto &item : it->value.GetArray())
        {
            if (!item.IsBool())
            {
                return default_value;
            }
            arr.push_back(item.GetBool());
        }
        return arr;
    }
    else
    {
        return default_value;
    }
}
bool OpenraspConfig::FromJson(const string &json)
{
    jsonObj = make_shared<rapidjson::Document>();
    jsonObj->Parse(json.c_str());
    if (jsonObj->HasParseError())
    {
        error_message = rapidjson::GetParseError_En(jsonObj->GetParseError());
        has_error = true;
        return false;
    }
    has_error = false;
    fromType = FromType::kJson;
    return true;
}
bool OpenraspConfig::FromIni(const string &ini)
{
    istringstream in(ini);
    auto parser = cpptoml::parser(in);
    try
    {
        tomlObj = parser.parse();
        has_error = false;
        fromType = FromType::kIni;
        return true;
    }
    catch (const cpptoml::parse_exception &ex)
    {
        has_error = true;
        error_message = ex.what();
        return false;
    }
    catch (...)
    {
        has_error = true;
        error_message = "Unknown Error";
        return false;
    }
}
bool OpenraspConfig::From(const string &config, FromType type)
{
    bool result = false;
    switch (type)
    {
    case FromType::kJson:
        result = FromJson(config);
        break;
    case FromType::kIni:
        result = FromIni(config);
        break;
    default:
        break;
    }
    return result;
}
} // namespace openrasp