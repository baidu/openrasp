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

#include "webdir_utils.h"
#include "utils/json_reader.h"

namespace openrasp
{
void sensitive_files_policy_alarm(std::map<std::string, std::vector<std::string>> &sensitive_file_map)
{
    for (auto &it : sensitive_file_map)
    {
        openrasp::JsonReader j;
        j.write_int64({"policy_id"}, 3009);
        j.write_string({"policy_params", "webroot"}, it.first);
        j.write_vector({"policy_params", "files"}, it.second);
        j.write_string({"message"}, "Multiple sensitive files found in " + it.first);
        LOG_G(policy_logger).log(LEVEL_INFO, j);
    }
}

} // namespace openrasp
