/*
 * Copyright 2017-2019 Baidu Inc.
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

extern "C"
{
#include "ext/pcre/php_pcre.h"
}

namespace openrasp
{

#if (PHP_MINOR_VERSION < 3)

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
    pcre_free(re);

    return rc >= 0;
}

#else

bool regex_match(const char *str, const char *regex, int options)
{
    int errornumber;
    PCRE2_SIZE erroroffset;

    pcre2_code *re = pcre2_compile(
        (PCRE2_SPTR)regex,     /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        options,               /* default options */
        &errornumber,          /* for error number */
        &erroroffset,          /* for error offset */
        nullptr);              /* use default compile context */

    /* Compilation failed: print the error message and exit. */

    if (re == nullptr)
    {
        return false;
    }

    /* Using this function ensures that the block is exactly the right size for
the number of capturing parentheses in the pattern. */

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, nullptr);

    int rc = pcre2_match(
        re,                    /* the compiled pattern */
        (PCRE2_SPTR)str,       /* the subject string */
        PCRE2_ZERO_TERMINATED, /* the length of the subject */
        0,                     /* start at offset 0 in the subject */
        0,                     /* default options */
        match_data,            /* block for storing the result */
        nullptr);              /* use default match context */

    /* Matching failed: handle error cases */
    pcre2_match_data_free(match_data); /* Release memory used for the match */
    pcre2_code_free(re);               /* data and the compiled pattern. */

    return rc >= 0;
}

#endif // (PHP_MINOR_VERSION < 3)

} // namespace openrasp