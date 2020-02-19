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

#include "regex.h"
#include "utils/regex.h"

namespace openrasp
{
namespace validator
{
namespace vstring
{

Regex::Regex(const std::string &regex, const std::string &error_description)
    : regex(regex), error_description(error_description)
{
}
std::string Regex::check(const std::string &value) const
{
    std::string result;
    if (!openrasp::regex_match(value.c_str(), regex.c_str()))
    {
        result = error_description;
    }
    return result;
}

} // namespace vstring

} // namespace validator

} // namespace openrasp
