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

#include "lower_limit.h"

namespace openrasp
{
namespace validator
{
namespace vint64
{

LowerLimit::LowerLimit(int64_t lower_limit, bool zero_valid)
{
    this->lower_limit = lower_limit;
    this->zero_valid = zero_valid;
}

std::string LowerLimit::check(const int64_t value) const
{
    std::string result;
    if (value < lower_limit)
    {
        result = "the value shoule be >= " + std::to_string(lower_limit);
        if (zero_valid && lower_limit > 0)
        {
            if (0 == value)
            {
                result.clear();
            }
            else
            {
                result += " OR = 0";
            }
        }
    }
    return result;
}

} // namespace vint64

} // namespace validator

} // namespace openrasp