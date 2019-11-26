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

#pragma once

#include "openrasp.h"
#include "builtin_material.h"
#include <cctype>

namespace openrasp
{
namespace data
{
class CallableMaterial : public BuiltinMaterial
{
protected:
    const std::vector<std::string> &callable_blacklist;

public:
    CallableMaterial(const std::vector<std::string> &callable_blacklist) : callable_blacklist(callable_blacklist)
    {
    }
    virtual bool is_valid() const = 0;
    virtual OpenRASPCheckType get_builtin_check_type() const = 0;
    virtual void fill_json_with_params(JsonReader &j) const = 0;
    virtual std::string get_function_name() const = 0;
    virtual bool builtin_check(JsonReader &j) const
    {
        bool result = false;
        std::string function_name = get_function_name();
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
            result = std::find(
                         callable_blacklist.begin(),
                         callable_blacklist.end(),
                         function_name) !=
                     callable_blacklist.end();
        }
        return result;
    }
};
} // namespace data

} // namespace openrasp