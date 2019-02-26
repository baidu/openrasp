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

#include "file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <limits>

namespace openrasp
{

bool file_exists(const std::string &file_path)
{
    return (access(file_path.c_str(), F_OK) == 0);
}

bool file_readable(const std::string &file_path)
{
    return (access(file_path.c_str(), F_OK | R_OK) == 0);
}

std::string get_line_content(const std::string &file_path, long num)
{
    std::string content;
    if (!file_readable(file_path) || num <= 0)
    {
        return content;
    }
    std::ifstream input(file_path);
    input.seekg(std::ios::beg);
    for (int i = 0; i < num - 1; ++i)
    {
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::getline(input, content);
    return content;
}

bool write_string_to_file(const char *file, std::ios_base::openmode mode, const char *content, size_t content_len)
{
    std::ofstream out_file(file, mode);
    if (out_file.is_open() && out_file.good())
    {
        out_file.write(content, content_len);
        out_file.close();
        return true;
    }
    return false;
}

} // namespace openrasp
