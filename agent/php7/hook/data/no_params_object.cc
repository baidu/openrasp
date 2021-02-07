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

#include "no_params_object.h"

namespace openrasp
{
namespace data
{

NoParamsObject::NoParamsObject(const OpenRASPCheckType check_type)
{
    this->check_type = check_type;
}
std::string NoParamsObject::build_lru_key() const
{
    return "";
}
OpenRASPCheckType NoParamsObject::get_v8_check_type() const
{
    return check_type;
}
bool NoParamsObject::is_valid() const
{
    return true;
}
void NoParamsObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    return;
}

} // namespace data

} // namespace openrasp