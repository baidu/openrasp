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

#include "webdir_detector.h"
#include <algorithm>

namespace openrasp
{

void WebDirDetector::insert_directory(std::string &path)
{
  std::lock_guard<std::mutex> lock(wdd_mutex);
  WebDir dir(path);
  if (std::find(webdirs.begin(), webdirs.end(), dir) == webdirs.end())
  {
    webdirs.push_back(dir);
  }
}

bool WebDirDetector::webdirs_composer_lock_modified()
{
  std::lock_guard<std::mutex> lock(wdd_mutex);
  bool result = false;
  for (const WebDir &dir : webdirs)
  {
    if (dir.is_composer_lock_modified())
    {
      result = true;
      break;
    }
  }
  return result;
}

std::vector<DependencyItem> WebDirDetector::dependency_detect()
{
  std::lock_guard<std::mutex> lock(wdd_mutex);
  std::vector<DependencyItem> all_deps;
  for (WebDir &dir : webdirs)
  {
    std::vector<DependencyItem> deps = dir.get_dependency();
    if (deps.size() > 0)
    {
      all_deps.insert(all_deps.end(), deps.begin(), deps.end());
    }
    dir.update_composer_lock_status();
  }
  return all_deps;
}

std::map<std::string, std::vector<std::string>> WebDirDetector::sensitive_file_detect(long scan_limit)
{
  std::lock_guard<std::mutex> lock(wdd_mutex);
  std::map<std::string, std::vector<std::string>> result;
  for (const WebDir &dir : webdirs)
  {
    std::vector<std::string> sensitive_files = dir.get_sensitive_files(scan_limit);
    if (sensitive_files.size() > 0)
    {
      result.insert({dir.get_abs_path(), sensitive_files});
    }
  }
  return result;
}

} // namespace openrasp
