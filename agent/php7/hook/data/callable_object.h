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

#pragma once

#include "builtin_material.h"
#include <unordered_set>

namespace openrasp
{
namespace data
{
class CallableObject : public BuiltinMaterial
{
protected:
    //do not efree here
    zval *function = nullptr;
    const std::unordered_set<std::string>& callable_blacklist;

public:
    CallableObject(zval *function, const std::unordered_set<std::string> &callable_blacklist);
    virtual bool is_valid() const;
    virtual OpenRASPCheckType get_builtin_check_type() const;
    virtual void fill_json_with_params(JsonReader &j) const;
    virtual bool builtin_check(JsonReader &j) const;
};

} // namespace data

} // namespace openrasp