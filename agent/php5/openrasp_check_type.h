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

#ifndef OPENRASP_CHECK_TYPE_H
#define OPENRASP_CHECK_TYPE_H

#include "openrasp.h"
#include <string>
#include <memory>
#include <set>

typedef enum check_type_t
{
  INVALID_TYPE = 0,
  CALLABLE,
  COMMAND,
  DIRECTORY,
  READ_FILE,
  WRITE_FILE,
  COPY,
  RENAME,
  FILE_UPLOAD,
  INCLUDE,
  DB_CONNECTION,
  SQL,
  SQL_SLOW_QUERY,
  SQL_PREPARED,
  SSRF,
  WEBSHELL_EVAL,
  WEBSHELL_COMMAND,
  WEBSHELL_FILE_PUT_CONTENTS,
  XSS_ECHO,
  ALL_TYPE
} OpenRASPCheckType;

class CheckTypeTransfer
{
private:
  std::map<OpenRASPCheckType, const std::string> check_type_to_name;
  std::map<const std::string, OpenRASPCheckType> name_to_check_type;
  std::vector<OpenRASPCheckType> buildin_check_type;
  void insert(OpenRASPCheckType type, const std::string &name, bool is_buildin = false);

public:
  CheckTypeTransfer();
  std::string type_to_name(OpenRASPCheckType type) const;
  OpenRASPCheckType name_to_type(std::string &name) const;
  std::vector<std::string> get_all_names() const;
  const std::vector<OpenRASPCheckType> &get_buildin_types() const;
};

extern std::unique_ptr<CheckTypeTransfer> check_type_transfer;

#endif