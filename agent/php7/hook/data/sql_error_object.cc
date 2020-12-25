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

#include "sql_error_object.h"
#include "utils/utf.h"
#include "agent/shared_config_manager.h"

namespace openrasp
{
namespace data
{

SqlErrorObject::SqlErrorObject(const V8Material &v8_material, const std::string &sql_type, const std::string &str_code, const std::string &error_msg)
    : v8_material(v8_material)
{
    this->sql_type = sql_type;
    this->str_code = str_code;
    this->error_msg = error_msg;
}

SqlErrorObject::SqlErrorObject(const V8Material &v8_material, const std::string &sql_type, long num_code, const std::string &error_msg)
    : v8_material(v8_material)
{
    this->sql_type = sql_type;
    this->num_code = num_code;
    this->error_msg = error_msg;
}

bool SqlErrorObject::is_valid() const
{
    if ("mysql" == sql_type)
    {
        return openrasp::scm->mysql_error_code_exist(num_code);
    }
    else if ("sqlite" == sql_type)
    {
        return openrasp::scm->sqlite_error_code_exist(num_code);
    }
    else if ("pgsql" == sql_type)
    {
        return openrasp::scm->pg_error_filtered(str_code);
    }
    return false;
}

//v8
std::string SqlErrorObject::build_lru_key() const
{
    return "";
}

OpenRASPCheckType SqlErrorObject::get_v8_check_type() const
{
    return SQL_ERROR;
}

void SqlErrorObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    auto context = isolate->GetCurrentContext();
    if ("pgsql" == sql_type)
    {
        params->Set(context, openrasp::NewV8String(isolate, "error_code"), openrasp::NewV8String(isolate, str_code)).IsJust();
    }
    else
    {
        params->Set(context, openrasp::NewV8String(isolate, "error_code"), openrasp::NewV8String(isolate, std::to_string(num_code))).IsJust();
    }
    std::string utf8_err_msg = openrasp::replace_invalid_utf8(error_msg);
    params->Set(context, openrasp::NewV8String(isolate, "error_msg"), openrasp::NewV8String(isolate, utf8_err_msg)).IsJust();
    return v8_material.fill_object_2b_checked(isolate, params);
}

} // namespace data

} // namespace openrasp