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

#include "check_utils.h"
#include "agent/shared_config_manager.h"
#include "agent/shared_log_manager.h"
#include <functional>

extern "C"
{
#ifdef HAVE_LINE_COVERAGE
    void __gcov_flush();
#endif
}

namespace openrasp
{

void builtin_alarm_info(openrasp::JsonReader &base_json)
{
    TSRMLS_FETCH();
    LOG_G(alarm_logger).log(LEVEL_INFO, base_json);
}

void builtin_policy_info(openrasp::JsonReader &base_json, ulong hash)
{
    TSRMLS_FETCH();
    bool skip = false;
    if (hash > 0 && slm != nullptr)
    {
        long timestamp = (long)time(nullptr);
        if (slm->log_update(timestamp, hash))
        {
            skip = true;
        }
    }
    if (!skip)
    {
        LOG_G(policy_logger).log(LEVEL_INFO, base_json);
    }
#ifdef HAVE_LINE_COVERAGE
    __gcov_flush();
#endif
}

CheckResult get_builtin_check_result(OpenRASPCheckType check_type)
{
    return (CheckResult)openrasp::scm->get_buildin_check_action(check_type);
}

} // namespace openrasp