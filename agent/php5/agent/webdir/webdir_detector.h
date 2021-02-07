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

#ifndef _OPENRASP_WEBDIR_DETECTOR_H_
#define _OPENRASP_WEBDIR_DETECTOR_H_

#include <string>
#include <vector>
#include <map>
#include "webdir.h"
#include <mutex>

namespace openrasp
{

class WebDirDetector
{
private:
  std::vector<WebDir> webdirs;
  std::mutex wdd_mutex;

public:
  void insert_directory(std::string &path);
  bool webdirs_composer_lock_modified();
  std::vector<DependencyItem> dependency_detect();
  std::map<std::string, std::vector<std::string>> sensitive_file_detect(long scan_limit);
};

} // namespace openrasp

#endif
