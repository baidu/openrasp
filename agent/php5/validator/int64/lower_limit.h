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

#include "base.h"

namespace openrasp
{
namespace validator
{
namespace vint64
{
class LowerLimit : public Base
{
private:
    int64_t lower_limit = 0;
    bool zero_valid = false;

public:
    LowerLimit() = default;
    LowerLimit(int64_t lower_limit, bool zero_valid = false);
    virtual std::string check(const int64_t value) const;
};
} // namespace vint64

} // namespace validator

} // namespace openrasp