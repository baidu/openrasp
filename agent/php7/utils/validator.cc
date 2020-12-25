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

#include "utils/validator.h"
#include "utils/regex.h"

namespace openrasp
{
std::string limit_int64(int64_t value, int64_t lower_limit, bool zero_valid)
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

std::string ge_zero_int64(int64_t value)
{
    return limit_int64(value, 0, false);
}

std::string g_zero_int64(int64_t value)
{
    return limit_int64(value, 1, false);
}

std::string nonempty_string(const std::string &value)
{
    std::string result;
    if (value.empty())
    {
        result = "the string value shoule not be empty.";
    }
    return result;
}

std::string regex_string(const std::string &value, const std::string &regex, const std::string &error_description)
{
    std::string result;
    if (!openrasp::regex_match(value.c_str(), regex.c_str()))
    {
        result = error_description;
    }
    return result;
}
} // namespace openrasp
