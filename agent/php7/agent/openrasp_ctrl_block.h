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

class OpenraspCtrlBlock
{
private:
  /*agent block*/
  pid_t supervisor_id = 0;
  pid_t log_agent_id = 0;
  pid_t master_pid = 0;
  pid_t plugin_agent_id = 0;
  pid_t webdir_agent_id = 0;
  bool registered = false;

public:
  /*agent block*/
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

  void set_registered(bool registered);
  bool get_registered();
};

} // namespace openrasp
