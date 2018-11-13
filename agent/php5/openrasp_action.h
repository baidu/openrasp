/*
 * Copyright 2017-2018 Baidu Inc.
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

#ifndef OPENRASP_ACTION_H
#define OPENRASP_ACTION_H

#include "openrasp.h"
#include <string>

typedef enum action_type_t
{
    AC_IGNORE = 0,
    AC_LOG = 1 << 0,
    AC_BLOCK = 1 << 1
} OpenRASPActionType;

inline OpenRASPActionType string_to_action(std::string action_string)
{
    return (action_string == "log") ? AC_LOG : ((action_string == "block") ? AC_BLOCK : AC_IGNORE);
}

#endif