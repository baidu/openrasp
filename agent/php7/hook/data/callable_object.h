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

#include "callable_material.h"

namespace openrasp
{
namespace data
{
class CallableObject : public CallableMaterial
{
protected:
    //do not efree here
    zval *function = nullptr;

public:
    CallableObject(zval *function, const std::vector<std::string> &callable_blacklist);
    virtual bool is_valid() const;
    virtual OpenRASPCheckType get_builtin_check_type() const;
    virtual void fill_json_with_params(JsonReader &j) const;
    virtual std::string get_function_name() const;
};

} // namespace data

} // namespace openrasp