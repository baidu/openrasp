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

#include "regex.h"
#include <regex>

namespace openrasp
{

bool regex_match(const char *str, const char *regex)
{
    try
    {
        const std::regex re(regex);
        return std::regex_match(str, re);
    }
    catch (std::regex_error &e)
    {
        //skip
    }
    return false;
}

bool regex_search(const char *str, const char *regex)
{
    try
    {
        const std::regex re(regex);
        return std::regex_search(str, re);
    }
    catch (std::regex_error &e)
    {
        //skip
    }
    return false;
}

} // namespace openrasp