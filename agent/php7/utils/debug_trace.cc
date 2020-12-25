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

#include "debug_trace.h"
#include "utils/file.h"

namespace openrasp
{

void DebugTrace::set_function(const std::string &function)
{
    this->function = function;
}

void DebugTrace::set_file(const std::string &file)
{
    this->file = file;
}

void DebugTrace::set_line(long line)
{
    this->line = line;
}

std::string DebugTrace::to_log_string() const
{
    return file + '@' + function + ':' + std::to_string(line);
}

std::string DebugTrace::get_source_code() const
{
    std::string source_code = get_line_content(file, line);
    if (!source_code.empty() && source_code.back() == '\r')
    {
        source_code.erase(source_code.length() - 1, 1);
    }
    return source_code;
}

} // namespace openrasp
