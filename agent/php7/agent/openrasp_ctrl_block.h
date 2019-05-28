/*
 * Copyright 2017-2019 Baidu Inc.
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

#ifndef _OPENRASP_CONTROL_BLOCK_
#define _OPENRASP_CONTROL_BLOCK_

#include <string>
#include "openrasp_hook.h"

namespace openrasp
{

class OpenraspCtrlBlock
{
private:
  static const int plugin_md5_size = 32;
  static const int plugin_name_size = 50;
  static const int plugin_version_size = 50;
  static const int webroot_max_size = 1 << 8;
  static const int agent_min_interval = 10;
  static const int agent_max_interval = 24 * 60 * 60;

public:
  static const int default_dependency_interval = 6 * 60 * 60;
  static const long default_scan_limit;

  void set_supervisor_id(pid_t supervisor_id);
  pid_t get_supervisor_id();

  void set_plugin_agent_id(pid_t plugin_agent_id);
  pid_t get_plugin_agent_id();

  void set_webdir_agent_id(pid_t webdir_agent_id);
  pid_t get_webdir_agent_id();

  void set_log_agent_id(pid_t log_agent_id);
  pid_t get_log_agent_id();

  void set_master_pid(pid_t master_pid);
  pid_t get_master_pid();

  void set_plugin_version(const char *plugin_version);
  const char *get_plugin_version();

  void set_plugin_name(const char *plugin_name);
  const char *get_plugin_name();

  void set_plugin_md5(const char *plugin_md5);
  const char *get_plugin_md5();

  long get_last_update_time();

  int get_webroot_count();
  void set_webroot_count(int webroot_count);

  bool webroot_found(ulong hash);
  void set_webroot_hash(int index, ulong hash);

  void set_webroot_path(const char *webroot_path);
  const char *get_webroot_path();

  int get_dependency_interval();
  void set_dependency_interval(int dependency_interval);

  long get_scan_limit();
  void set_scan_limit(long scan_limit);

private:
  pid_t supervisor_id = 0;
  pid_t log_agent_id = 0;
  pid_t master_pid = 0;
  pid_t plugin_agent_id = 0;
  pid_t webdir_agent_id = 0;

  long last_update_time = 0;

  long scan_limit = OpenraspCtrlBlock::default_scan_limit;
  int dependency_interval = OpenraspCtrlBlock::default_dependency_interval;
  int webroot_count = 0;
  ulong webroot_hash[OpenraspCtrlBlock::webroot_max_size];
  char webroot_path[MAXPATHLEN] = {0};

  char plugin_md5[OpenraspCtrlBlock::plugin_md5_size + 1] = {0};
  char plugin_name[OpenraspCtrlBlock::plugin_name_size + 1] = {0};
  char plugin_version[OpenraspCtrlBlock::plugin_version_size + 1] = {0};
};

} // namespace openrasp

#endif
