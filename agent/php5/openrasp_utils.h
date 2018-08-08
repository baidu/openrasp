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

#ifndef OPENRASP_UTILS_H
#define OPENRASP_UTILS_H

#include "openrasp.h"
#include <string>
#include <vector>
#include <functional>

long fetch_time_offset(TSRMLS_D);
const char *fetch_url_scheme(const char *filename);
void format_debug_backtrace_arr(zval *backtrace_arr TSRMLS_DC);
void format_debug_backtrace_str(zval *backtrace_str TSRMLS_DC);
int recursive_mkdir(const char *path, int len, int mode TSRMLS_DC);
bool same_day_in_current_timezone(long src, long target, long offset);
void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins, std::function<bool(const char *filename)> file_filter);

#endif