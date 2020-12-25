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

#ifndef _OPENRASP_UTILS_VALIDATOR_H_
#define _OPENRASP_UTILS_VALIDATOR_H_

#include <string>

namespace openrasp
{
std::string limit_int64(int64_t value, int64_t lower_limit, bool zero_valid);
std::string ge_zero_int64(int64_t value);
std::string g_zero_int64(int64_t value);
std::string nonempty_string(const std::string &value);
std::string regex_string(const std::string &value, const std::string &regex, const std::string &error_description);

} // namespace openrasp

#endif