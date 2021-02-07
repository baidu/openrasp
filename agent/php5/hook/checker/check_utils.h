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

#pragma once

#include "openrasp_log.h"
#include "check_result.h"
#include "openrasp_check_type.h"

namespace openrasp
{

void builtin_alarm_info(openrasp::JsonReader &base_json);
void builtin_policy_info(openrasp::JsonReader &base_json, ulong hash = 0);
CheckResult get_builtin_check_result(OpenRASPCheckType check_type);

} // namespace openrasp