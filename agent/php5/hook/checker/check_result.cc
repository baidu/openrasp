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

#include "check_result.h"

namespace openrasp
{

CheckResult string_to_check_result(std::string action_string)
{
    return (action_string == "block") ? kBlock : ((action_string == "ignore") ? kNoCache : kLog);
}

std::string check_result_to_string(CheckResult type)
{
    return (kBlock == type) ? "block" : ((kLog == type) ? "log" : "ignore");
}

} // namespace openrasp