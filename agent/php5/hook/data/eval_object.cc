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

#include "eval_object.h"

namespace openrasp
{
namespace data
{

EvalObject::EvalObject(zval *code, const std::string &function) : function(function)
{
    this->code = code;
}

bool EvalObject::is_valid() const
{
    if (nullptr == code || Z_TYPE_P(code) != IS_STRING || function.empty())
    {
        return false;
    }
    return true;
}

//v8
std::string EvalObject::build_lru_key() const
{
    return "";
}
OpenRASPCheckType EvalObject::get_v8_check_type() const
{
    return EVAL;
}
void EvalObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "code"), openrasp::NewV8String(isolate, Z_STRVAL_P(code), Z_STRLEN_P(code))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function)).IsJust();
}

//builtin
void EvalObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"attack_params", "eval"}, std::string(Z_STRVAL_P(code), Z_STRLEN_P(code)));
    j.write_string({"plugin_message"}, "WebShell activity - Detected China Chopper webshell (" + function + " method)");
}

OpenRASPCheckType EvalObject::get_builtin_check_type() const
{
    return WEBSHELL_EVAL;
}
std::vector<zval *> EvalObject::get_zval_ptrs() const
{
    return {code};
}

} // namespace data

} // namespace openrasp