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

#include "openrasp_check_type.h"

void CheckTypeTransfer::insert(OpenRASPCheckType type, const std::string &name, bool is_buildin)
{
  check_type_to_name.insert({type, name});
  name_to_check_type.insert({name, type});
  if (is_buildin)
  {
    buildin_check_type.push_back(type);
  }
}

CheckTypeTransfer::CheckTypeTransfer()
{
  insert(CALLABLE, "webshell_callable", true);
  insert(COMMAND, "command");
  insert(DIRECTORY, "directory");
  insert(READ_FILE, "readFile");
  insert(WRITE_FILE, "writeFile");
  insert(COPY, "copy");
  insert(RENAME, "rename");
  insert(FILE_UPLOAD, "fileUpload");
  insert(INCLUDE, "include");
  insert(DB_CONNECTION, "dbConnection");
  insert(SQL, "sql");
  insert(SQL_SLOW_QUERY, "sqlSlowQuery");
  insert(SQL_PREPARED, "sqlPrepared");
  insert(SSRF, "ssrf");
  insert(WEBSHELL_EVAL, "webshell_eval", true);
  insert(WEBSHELL_COMMAND, "webshell_command", true);
  insert(WEBSHELL_FILE_PUT_CONTENTS, "webshell_file_put_contents", true);
  insert(XSS_ECHO, "xss_echo", true);
}

std::string CheckTypeTransfer::type_to_name(OpenRASPCheckType type) const
{
  auto it = check_type_to_name.find(type);
  if (it != check_type_to_name.end())
  {
    return it->second;
  }
  else
  {
    return "unknown";
  }
}

OpenRASPCheckType CheckTypeTransfer::name_to_type(std::string &name) const
{
  auto it = name_to_check_type.find(name);
  if (it != name_to_check_type.end())
  {
    return it->second;
  }
  else
  {
    return INVALID_TYPE;
  }
}

std::vector<std::string> CheckTypeTransfer::get_all_names() const
{
  std::vector<std::string> names;
  for (auto &item : name_to_check_type)
  {
    names.push_back(item.first);
  }
  return names;
}

const std::vector<OpenRASPCheckType>& CheckTypeTransfer::get_buildin_types() const
{
  return buildin_check_type;
}

std::unique_ptr<CheckTypeTransfer> check_type_transfer(new CheckTypeTransfer());