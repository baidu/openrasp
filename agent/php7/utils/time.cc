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

#include "time.h"
#include <string>
#include <math.h>

namespace openrasp
{

long fetch_time_offset()
{
    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);
    return lt.tm_gmtoff;
}

bool same_day_in_current_timezone(long src, long target, long offset)
{
    long day = 24 * 60 * 60;
    return ((src + offset) / day == (target + offset) / day);
}

std::string format_time(const char *format, int format_len, time_t ts)
{
    char buffer[128];
    struct tm *tm_info;

    tm_info = localtime(&ts);

    strftime(buffer, 64, format, tm_info);
    return std::string(buffer);
}

unsigned long increase_interval_by_factor(unsigned long origin, double factor, unsigned long max)
{
    if (origin >= max)
    {
        return max;
    }
    factor = factor >= 0 ? factor : (0 - factor);
    unsigned long result = floor(origin * factor);
    return result > max ? max : result;
}

} // namespace openrasp
