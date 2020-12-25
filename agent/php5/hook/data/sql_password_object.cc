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

#include "agent/shared_config_manager.h"
#include "openrasp_log.h"
#include "sql_password_object.h"
#include <cctype>
#include <sstream>
#include <map>

namespace openrasp
{
namespace data
{

SqlPasswordObject::SqlPasswordObject(const SqlConnectionObject &sql_connection_object)
    : sql_connection_object(sql_connection_object)
{
}

bool SqlPasswordObject::is_valid() const
{
    if (sql_connection_object.is_valid())
    {
        if (slm != nullptr && slm->log_exist((long)time(nullptr), hash()))
        {
            return false;
        }
        return true;
    }
    return false;
}

void SqlPasswordObject::fill_json_with_params(JsonReader &j) const
{
    std::ostringstream oss;
    oss << "Database security baseline - weak password detected for \""
        << sql_connection_object.get_username()
        << "\" account, password is: \""
        << sql_connection_object.get_password()
        << "\"";
    j.write_int64({"policy_id"}, 3003);
    j.write_string({"message"}, oss.str());
    j.write_string({"policy_params", "password"}, sql_connection_object.get_password());
    return sql_connection_object.fill_json_with_params(j);
}

bool SqlPasswordObject::policy_check(JsonReader &j) const
{
    return openrasp::scm->is_password_weak(sql_connection_object.get_password());
}

ulong SqlPasswordObject::hash() const
{
    return std::hash<std::string>{}("password" + sql_connection_object.build_lru_key());
}

} // namespace data

} // namespace openrasp