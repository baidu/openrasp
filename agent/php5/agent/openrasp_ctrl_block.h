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
namespace openrasp
{

class OpenraspCtrlBlock
{
private:
  static const int plugin_md5_size = 32;
  static const int plugin_name_size = 50;
  static const int plugin_version_size = 50;

public:
  void set_supervisor_id(pid_t supervisor_id);
  pid_t get_supervisor_id();

  void set_plugin_agent_id(pid_t plugin_agent_id);
  pid_t get_plugin_agent_id();

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

private:
  pid_t supervisor_id = 0;
  pid_t log_agent_id = 0;
  pid_t master_pid = 0;
  pid_t plugin_agent_id = 0;

  long last_update_time = 0;
  char plugin_md5[OpenraspCtrlBlock::plugin_md5_size + 1] = {0};
  char plugin_name[OpenraspCtrlBlock::plugin_name_size + 1] = {0};
  char plugin_version[OpenraspCtrlBlock::plugin_version_size + 1] = {0};
};

} // namespace openrasp

#endif
