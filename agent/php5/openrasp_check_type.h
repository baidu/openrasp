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

#ifndef OPENRASP_CHECK_TYPE_H
#define OPENRASP_CHECK_TYPE_H

#include "openrasp.h"
#include <string>
#include <memory>
#include <map>

enum OpenRASPCheckType
{
  INVALID_TYPE = 0,
  CALLABLE,
  COMMAND,
  DIRECTORY,
  READ_FILE,
  WRITE_FILE,
  DELETE_FILE,
  COPY,
  RENAME,
  FILE_UPLOAD,
  INCLUDE,
  EVAL,
  DB_CONNECTION,
  SQL,
  SQL_PREPARED,
  SQL_ERROR,
  SSRF,
  WEBSHELL_EVAL,
  WEBSHELL_COMMAND,
  WEBSHELL_FILE_PUT_CONTENTS,
  WEBSHELL_ENV,
  XSS_ECHO,
  XSS_USER_INPUT,
  REQUEST,
  REQUEST_END,
  MONGO,
  SSRF_REDIRECT,
  RESPONSE,
  ALL_TYPE
};

class CheckTypeTransfer
{
private:
  std::map<OpenRASPCheckType, const std::string> check_type_to_name;
  std::map<const std::string, OpenRASPCheckType> name_to_check_type;
  std::vector<OpenRASPCheckType> buildin_check_type;
  void insert(OpenRASPCheckType type, const std::string &name, bool is_buildin = false);

  CheckTypeTransfer();
  virtual ~CheckTypeTransfer();
  CheckTypeTransfer(const CheckTypeTransfer &) = delete;
  CheckTypeTransfer &operator=(const CheckTypeTransfer &) = delete;

public:
  static CheckTypeTransfer &instance();
  //only read op
  std::string type_to_name(OpenRASPCheckType type) const;
  OpenRASPCheckType name_to_type(const std::string &name) const;
  std::map<std::string, std::string> get_buildin_action_map() const;
  std::vector<OpenRASPCheckType> get_buildin_check_types() const;
};

#endif