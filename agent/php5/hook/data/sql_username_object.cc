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

#include "openrasp_log.h"
#include "sql_username_object.h"
#include <cctype>
#include <sstream>
#include <map>

namespace openrasp
{
namespace data
{

SqlUsernameObject::SqlUsernameObject(const SqlConnectionObject &sql_connection_object)
    : sql_connection_object(sql_connection_object)
{
}

bool SqlUsernameObject::is_valid() const
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

void SqlUsernameObject::fill_json_with_params(JsonReader &j) const
{
    std::ostringstream oss;
    oss << "Database security - Connecting to a "
        << sql_connection_object.get_server()
        << " instance using the high privileged account: "
        << sql_connection_object.get_username();
    if (sql_connection_object.get_using_socket())
    {
        oss << " (via unix domain socket)";
    }
    j.write_int64({"policy_id"}, 3006);
    j.write_string({"message"}, oss.str());
    return sql_connection_object.fill_json_with_params(j);
}

bool SqlUsernameObject::policy_check(JsonReader &j) const
{
    static const std::multimap<std::string, std::string> database_username_blacklists = {
        {"mysql", "root"},
        {"mssql", "sa"},
        {"pgsql", "postgres"},
        {"oci", "dbsnmp"},
        {"oci", "sysman"},
        {"oci", "system"},
        {"oci", "sys"}};
    if (!sql_connection_object.get_username().empty())
    {
        auto pos = database_username_blacklists.equal_range(sql_connection_object.get_server());
        while (pos.first != pos.second)
        {
            if (sql_connection_object.get_username() == pos.first->second)
            {
                return true;
            }
            pos.first++;
        }
    }
    return false;
}

ulong SqlUsernameObject::hash() const
{
    return std::hash<std::string>{}("username" + sql_connection_object.build_lru_key());
}

} // namespace data

} // namespace openrasp