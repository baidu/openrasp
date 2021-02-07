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

#include "command_object.h"

namespace openrasp
{
namespace data
{

CommandObject::CommandObject(zval *command)
    : command(command)
{
}

bool CommandObject::is_valid() const
{
    return nullptr != command &&
           Z_TYPE_P(command) == IS_STRING &&
           Z_STRLEN_P(command) > 0;
}

//v8
std::string CommandObject::build_lru_key() const
{
    return "";
}
OpenRASPCheckType CommandObject::get_v8_check_type() const
{
    return COMMAND;
}
void CommandObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "command"), openrasp::NewV8String(isolate, Z_STRVAL_P(command), Z_STRLEN_P(command))).IsJust();
}

//builtin
void CommandObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"attack_params", "command"}, std::string(Z_STRVAL_P(command), Z_STRLEN_P(command)));
    j.write_string({"plugin_message"}, "WebShell activity - Detected command execution backdoor");
}

OpenRASPCheckType CommandObject::get_builtin_check_type() const
{
    return WEBSHELL_COMMAND;
}

std::vector<zval *> CommandObject::get_zval_ptrs() const
{
    return {command};
}

} // namespace data

} // namespace openrasp