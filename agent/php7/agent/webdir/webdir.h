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

#ifndef _OPENRASP_WEBDIR_H_
#define _OPENRASP_WEBDIR_H_

#include <string>
#include <vector>
#include "dependency_item.h"

namespace openrasp
{

class WebDir
{
private:
  std::string abs_path;
  time_t composer_lock_last_modified = 0;
  std::string composer_lock_hash;
  std::string get_composer_lock_path() const;
  std::string get_composer_lock_hash() const;

public:
  WebDir(std::string abs_path);
  bool operator<(const WebDir &rhs) const;
  bool is_composer_lock_modified() const;
  std::vector<std::string> get_sensitive_files(long scan_limit) const;
  std::string get_abs_path() const;
  std::vector<DependencyItem> get_dependency() const;
  void update_composer_lock_status();
  friend bool operator==(const WebDir &left, const WebDir &right);
};

} // namespace openrasp

#endif
