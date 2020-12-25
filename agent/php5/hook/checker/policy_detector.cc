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


#include "policy_detector.h"

namespace openrasp
{
namespace checker
{

bool PolicyDetector::pretreat() const
{
    if (!policy_material.is_valid())
    {
        return false;
    }
    return true;
}

CheckResult PolicyDetector::check()
{
    return (policy_material.policy_check(j)) ? kLog : kNoCache;
}

PolicyDetector::PolicyDetector(const openrasp::data::PolicyMaterial &policy_material)
    : policy_material(policy_material)
{
}

void PolicyDetector::run()
{
    if (!pretreat())
    {
        return;
    }
    if (kNoCache != check())
    {
        policy_material.fill_json_with_params(j);
        log_policy();
    }
}

void PolicyDetector::log_policy()
{
    j.write_vector({"policy_params", "stack"}, format_debug_backtrace_arr());
    builtin_policy_info(j, policy_material.hash());
}
} // namespace checker

} // namespace openrasp