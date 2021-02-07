/*
 * Copyright 2017-2021 Baidu Inc.
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

#include "callable_object.h"

namespace openrasp
{
namespace data
{

CallableObject::CallableObject(zval *function, const std::unordered_set<std::string> &callable_blacklist)
    : callable_blacklist(callable_blacklist)
{
    this->function = function;
}

bool CallableObject::is_valid() const
{
    return nullptr != function &&
           Z_TYPE_P(function) == IS_STRING &&
           Z_STRLEN_P(function) > 0;
}

OpenRASPCheckType CallableObject::get_builtin_check_type() const
{
    return CALLABLE;
}

void CallableObject::fill_json_with_params(JsonReader &j) const
{
    std::string function_name = std::string(Z_STRVAL_P(function), Z_STRLEN_P(function));
    j.write_string({"attack_params", "function"}, function_name);
    j.write_string({"plugin_message"}, "WebShell activity - Using dangerous callback method: " + function_name);
}

bool CallableObject::builtin_check(JsonReader &j) const
{
    bool result = false;
    std::string function_name = std::string(Z_STRVAL_P(function), Z_STRLEN_P(function));
    for (auto &ch : function_name)
    {
        ch = std::tolower(ch);
    }
    if (function_name.length() > 0)
    {
        if (function_name[0] == '\\')
        {
            function_name = function_name.substr(1);
        }
        auto found = callable_blacklist.find(function_name);
        result = (found != callable_blacklist.end());
    }
    return result;
}

} // namespace data

} // namespace openrasp