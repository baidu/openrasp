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

#include "utils/regex.h"
#include "echo_object.h"

namespace openrasp
{
namespace data
{

EchoObject::EchoObject(zval *value, const std::string &opcode_name, const std::string &regex)
: opcode_name(opcode_name), regex(regex)
{
    this->value = value;
}
bool EchoObject::is_valid() const
{
    if (nullptr == value || Z_TYPE_P(value) != IS_STRING)
    {
        return false;
    }
    if (opcode_name.empty())
    {
        return false;
    }
    return true;
}
void EchoObject::fill_json_with_params(JsonReader &j) const
{
    std::string type;
    std::string name;
    fetch_name_in_request(value, name, type);
    j.write_string({"attack_params", "type"}, type);
    j.write_string({"attack_params", "name"}, name);
    j.write_string({"attack_params", "value"}, Z_STRVAL_P(value));
    j.write_string({"plugin_message"}, "XSS activity - " + opcode_name +
                                           " GET/POST/COOKIE parameter directly, parameter: $" +
                                           type + "['" + name + "']");
}

OpenRASPCheckType EchoObject::get_builtin_check_type() const
{
    return XSS_ECHO;
}
bool EchoObject::builtin_check(JsonReader &j) const
{
    if (!regex.empty() && openrasp::regex_search(Z_STRVAL_P(value), regex.c_str()))
    {
        return true;
    }
    return false;
}

} // namespace data

} // namespace openrasp