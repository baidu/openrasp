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

#ifndef _OPENRASP_UTILS_DEBUG_TRACE_H_
#define _OPENRASP_UTILS_DEBUG_TRACE_H_

#include <string>
#include <vector>
#include <map>

namespace openrasp
{

class DebugTrace
{
  private:
    std::string function;
    std::string file;
    long line = -1;

  public:
    void set_function(const std::string &function);
    void set_file(const std::string &file);
    void set_line(long line);
    std::string to_log_string() const;
    std::string get_source_code() const;
};

} // namespace openrasp

#endif
