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

#ifndef _OPENRASP_UTILS_FILE_H_
#define _OPENRASP_UTILS_FILE_H_

#include <string>
#include <fstream>
#include <vector>
#include <functional>
#include <climits>

namespace openrasp
{

bool file_exists(const std::string &file_path);
bool file_readable(const std::string &file_path);
std::string get_line_content(const std::string &file_path, long num);
bool write_string_to_file(const char *file, std::ios_base::openmode mode, const char *content, size_t content_len);
bool read_entire_content(const std::string &file, std::string &content);
void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins,
                      std::function<bool(const char *filename)> file_filter, long limit = LONG_MAX,
                      bool use_abs_path = false, std::string default_slash = "/");
time_t get_last_modified(const std::string &file_path);

} // namespace openrasp

#endif
