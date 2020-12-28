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

#pragma once

#include <string>
#include "openrasp_hook.h"

namespace openrasp
{

class WebdirCtrlBlock
{
private:
  /*webdir block*/
  static const int webdir_scan_regex_size = 100;
  static const int webroot_max_size = 1 << 8;
  static const int agent_min_interval = 60;
  static const int agent_max_interval = 24 * 60 * 60;

  long scan_limit = WebdirCtrlBlock::default_scan_limit;
  int dependency_interval = WebdirCtrlBlock::default_dependency_interval;
  int webdir_scan_interval = WebdirCtrlBlock::default_webdir_scan_interval;
  int webroot_count = 0;
  ulong webroot_hash[WebdirCtrlBlock::webroot_max_size];
  char webroot_path[MAXPATHLEN] = {0};
  char webdir_scan_regex[WebdirCtrlBlock::webdir_scan_regex_size + 1] = {0};

public:
  /*webdir block*/
  static const int default_dependency_interval = 6 * 60 * 60;
  static const int default_webdir_scan_interval = 6 * 60 * 60;
  static const long default_scan_limit;
  static const std::string default_scan_regex;

  void set_webdir_scan_regex(const char *webdir_scan_regex);
  const char *get_webdir_scan_regex();

  int get_webroot_count();
  void set_webroot_count(int webroot_count);

  bool webroot_found(ulong hash);
  void set_webroot_hash(int index, ulong hash);

  void set_webroot_path(const char *webroot_path);
  const char *get_webroot_path();

  int get_dependency_interval();
  void set_dependency_interval(int dependency_interval);

  int get_webdir_scan_interval();
  void set_webdir_scan_interval(int webdir_scan_interval);

  long get_scan_limit();
  void set_scan_limit(long scan_limit);
};

} // namespace openrasp
