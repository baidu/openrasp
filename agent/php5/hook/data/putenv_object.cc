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

#include "putenv_object.h"
#include "agent/shared_config_manager.h"

namespace openrasp
{
namespace data
{

PutenvObject::PutenvObject(zval *env)
{
    this->env = env;
}
bool PutenvObject::is_valid() const
{
    return nullptr != env &&
           Z_TYPE_P(env) == IS_STRING &&
           Z_STRLEN_P(env) > 0;
}
void PutenvObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"attack_params", "env"}, std::string(Z_STRVAL_P(env), Z_STRLEN_P(env)));
    j.write_string({"plugin_message"}, "WebShell activity - using putenv() method");
}

OpenRASPCheckType PutenvObject::get_builtin_check_type() const
{
    return WEBSHELL_ENV;
}
bool PutenvObject::builtin_check(JsonReader &j) const
{
    return openrasp::scm->filter_env_key(std::string(Z_STRVAL_P(env)));
}

} // namespace data

} // namespace openrasp