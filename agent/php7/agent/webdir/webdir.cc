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

#include "webdir.h"
#include "utils/file.h"
#include "utils/regex.h"
#include "utils/json_reader.h"
#include "agent/openrasp_agent_manager.h"

namespace openrasp
{
WebDir::WebDir(std::string abs_path)
{
  this->abs_path = abs_path;
}

bool WebDir::operator<(const WebDir &rhs) const
{
  return abs_path < rhs.abs_path;
}

std::string WebDir::get_composer_lock_hash() const
{
  std::string composer_lock_path = get_composer_lock_path();
  std::string composer_lock_content;
  std::string content_hash;
  if (file_readable(composer_lock_path) &&
      read_entire_content(composer_lock_path, composer_lock_content))
  {
    JsonReader jReader(composer_lock_content);
    content_hash = jReader.fetch_string({"content-hash"}, content_hash);
  }
  return content_hash;
}

bool WebDir::is_composer_lock_modified() const
{
  return this->composer_lock_last_modified != get_last_modified(get_composer_lock_path()) ||
         this->composer_lock_hash != get_composer_lock_hash();
}

std::string WebDir::get_composer_lock_path() const
{
  return this->abs_path + "/composer.lock";
}

std::vector<std::string> WebDir::get_sensitive_files(long scan_limit) const
{
  std::vector<std::string> sensitive_files;
  openrasp_scandir(abs_path, sensitive_files,
                   [](const char *filename) {
                     return regex_search(filename, oam->get_webdir_scan_regex());
                   },
                   scan_limit);
  return sensitive_files;
}

std::string WebDir::get_abs_path() const
{
  return abs_path;
}

std::vector<DependencyItem> WebDir::get_dependency() const
{
  std::vector<DependencyItem> deps;
  std::string composer_lock_path = get_composer_lock_path();
  std::string composer_lock_content;
  if (file_readable(composer_lock_path) &&
      read_entire_content(composer_lock_path, composer_lock_content))
  {
    JsonReader jReader(composer_lock_content);
    size_t count = jReader.get_array_size({"packages"});
    for (size_t i = 0; i < count; ++i)
    {
      std::string name = jReader.fetch_string({"packages", std::to_string(i), "name"}, "");
      std::string version = jReader.fetch_string({"packages", std::to_string(i), "version"}, "");
      deps.push_back(DependencyItem(get_abs_path(), name, version));
    }
  }
  return deps;
}

void WebDir::update_composer_lock_status()
{
  this->composer_lock_last_modified != get_last_modified(get_composer_lock_path());
  this->composer_lock_hash != get_composer_lock_hash();
}

bool operator==(const WebDir &left, const WebDir &right)
{
  return left.get_abs_path() == right.get_abs_path();
}

} // namespace openrasp
