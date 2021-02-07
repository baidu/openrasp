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

#include "openrasp.h"
#include "builtin_material.h"
#include <cctype>
#include <vector>

namespace openrasp
{
namespace data
{
class WebshellMaterial : public BuiltinMaterial
{
public:
    virtual bool is_valid() const = 0;
    virtual OpenRASPCheckType get_builtin_check_type() const = 0;
    virtual void fill_json_with_params(JsonReader &j) const = 0;
    virtual std::vector<zval *> get_zval_ptrs() const = 0;
    virtual bool builtin_check(JsonReader &j) const
    {
        std::vector<zval *> zval_ptrs = get_zval_ptrs();
        for (zval *zval_ptr : zval_ptrs)
        {
            if (!openrasp_zval_in_request(zval_ptr))
            {
                return false;
            }
        }
        return true;
    }
};
} // namespace data

} // namespace openrasp