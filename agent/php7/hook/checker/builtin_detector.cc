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

#include "builtin_detector.h"
#include "check_utils.h"

namespace openrasp
{
namespace checker
{

bool BuiltinDetector::pretreat() const
{
    if (kNoCache == get_builtin_check_result(builtin_material.get_builtin_check_type()))
    {
        return false;
    }
    if (!builtin_material.is_valid())
    {
        return false;
    }
    return true;
}

CheckResult BuiltinDetector::check()
{
    CheckResult cr = kNoCache;
    if (builtin_material.builtin_check(j))
    {
        cr = get_builtin_check_result(builtin_material.get_builtin_check_type());
    }
    return cr;
}

BuiltinDetector::BuiltinDetector(const openrasp::data::BuiltinMaterial &builtin_material)
    : builtin_material(builtin_material)
{
}
void BuiltinDetector::run()
{
    if (!pretreat())
    {
        return;
    }
    CheckResult cr = check();
    if (kNoCache == cr)
    {
        return;
    }
    else
    {
        builtin_material.fill_json_with_params(j);
        log_alarm(cr);
        if (kBlock == cr)
        {
            block_handle();
        }
    }
}

void BuiltinDetector::log_alarm(const CheckResult &cr)
{
    j.write_int64({"plugin_confidence"}, 100);
    j.write_string({"plugin_name"}, "php_builtin_plugin");
    j.write_string({"attack_type"}, CheckTypeTransfer::instance().type_to_name(builtin_material.get_builtin_check_type()));
    j.write_string({"plugin_algorithm"}, CheckTypeTransfer::instance().type_to_name(builtin_material.get_builtin_check_type()));
    j.write_string({"intercept_state"}, check_result_to_string(cr));
    j.write_vector({"attack_params", "stack"}, format_debug_backtrace_arr());
    builtin_alarm_info(j);
}

} // namespace checker

} // namespace openrasp