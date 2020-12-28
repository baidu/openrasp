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

#include "xss_userinput_object.h"

namespace openrasp
{
namespace data
{

XssUserInputObject::XssUserInputObject(const std::string &name, zval *value)
    : name(name)
{
    this->value = value;
}
bool XssUserInputObject::is_valid() const
{
    return nullptr != value && Z_TYPE_P(value) == IS_STRING && !name.empty();
}
void XssUserInputObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"attack_params", "type"}, "_GET");
    j.write_string({"attack_params", "name"}, name);
    j.write_string({"attack_params", "value"}, std::string(Z_STRVAL_P(value), Z_STRLEN_P(value)));
    j.write_string({"plugin_message"}, "Reflected XSS attack detected: parameter: $_GET['" + name + "']");
}

OpenRASPCheckType XssUserInputObject::get_builtin_check_type() const
{
    return XSS_USER_INPUT;
}
bool XssUserInputObject::builtin_check(JsonReader &j) const
{
    //check in output handler
    return true;
}

} // namespace data

} // namespace openrasp