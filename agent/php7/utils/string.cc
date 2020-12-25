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

#include "string.h"
#include <cctype>
#include <algorithm>
namespace openrasp
{

bool start_with(const std::string &str, const std::string &prefix)
{
    size_t len1 = str.length();
    size_t len2 = prefix.length();
    if (len1 < len2)
    {
        return false;
    }
    return (!str.compare(0, len2, prefix));
}

bool end_with(const std::string &str, const std::string &suffix)
{
    size_t len1 = str.length();
    size_t len2 = suffix.length();
    if (len1 < len2)
    {
        return false;
    }
    return (!str.compare(len1 - len2, len2, suffix));
}

void string_replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

bool empty(const char *str)
{
    if (nullptr == str || strcmp(str, "") == 0)
    {
        return true;
    }
    return false;
}

bool case_insens_equal(std::string &str1, std::string &str2)
{
    return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), [](char &c1, char &c2) {
                return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
            }));
}


} // namespace openrasp
