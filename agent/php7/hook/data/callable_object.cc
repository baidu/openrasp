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

#include "callable_object.h"

namespace openrasp
{
namespace data
{

CallableObject::CallableObject(zval *function, const std::vector<std::string> &callable_blacklist)
    : CallableMaterial(callable_blacklist)
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
    j.write_string({"attack_params", "function"}, get_function_name());
    j.write_string({"plugin_message"}, "WebShell activity - Using dangerous callback method: " + get_function_name());
}

std::string CallableObject::get_function_name() const
{
    return std::string(Z_STRVAL_P(function), Z_STRLEN_P(function));
}

} // namespace data

} // namespace openrasp