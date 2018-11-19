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

#include "openrasp.h"
#include "regex.h"
#include <string.h>

extern "C"
{
#include "ext/pcre/php_pcre.h"
}

namespace openrasp
{

const static size_t OVECCOUNT = 30; /* should be a multiple of 3 */

bool regex_match(const char *str, const char *regex, int options)
{
    pcre *re = nullptr;
    int ovector[OVECCOUNT];
    const char *error = nullptr;
    int erroffset = 0;
    int rc = 0;
    re = pcre_compile(regex, options, &error, &erroffset, nullptr);
    if (re == nullptr)
    {
        return false;
    }
    rc = pcre_exec(re, nullptr, str, strlen(str), 0, 0, ovector, OVECCOUNT);
    if (rc < 0)
    {
        pcre_free(re);
        return false;
    }
    pcre_free(re);
    return true;
}
} // namespace openrasp