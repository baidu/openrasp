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

#include "webdir_utils.h"

namespace openrasp
{
void sensitive_files_policy_alarm(std::map<std::string, std::vector<std::string>> &sensitive_file_map)
{
    for (auto &it : sensitive_file_map)
    {
        TSRMLS_FETCH();
        zval *result = nullptr;
        MAKE_STD_ZVAL(result);
        array_init(result);
        add_assoc_long(result, "policy_id", 3008);
        zval *policy_params = nullptr;
        MAKE_STD_ZVAL(policy_params);
        array_init(policy_params);
        add_assoc_string(policy_params, "webroot", const_cast<char *>(it.first.c_str()), 1);
        zval *z_compressions = nullptr;
        MAKE_STD_ZVAL(z_compressions);
        array_init(z_compressions);
        for (auto &file : it.second)
        {
            add_next_index_string(z_compressions, const_cast<char *>(file.c_str()), 1);
        }
        add_assoc_zval(policy_params, "compressed_files", z_compressions);
        add_stack_to_params(policy_params TSRMLS_CC);
        add_assoc_zval(result, "policy_params", policy_params);
        add_assoc_string(result, "message", const_cast<char *>(("Compressed files found in webroot path:" + it.first).c_str()), 1);
        LOG_G(policy_logger).log(LEVEL_INFO, result TSRMLS_CC);
        zval_ptr_dtor(&result);
    }
}

} // namespace openrasp
