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

#ifndef _OPENRASP_AGENT_UTILS_TIME_H_
#define _OPENRASP_AGENT_UTILS_TIME_H_

#include <string>
#include <time.h>

namespace openrasp
{

long fetch_time_offset();
bool same_day_in_current_timezone(long src, long target, long offset);
std::string format_time(const char *format, int format_len, time_t ts);

unsigned long increase_interval_by_factor(unsigned long origin, double factor, unsigned long max);

} // namespace openrasp

#endif
