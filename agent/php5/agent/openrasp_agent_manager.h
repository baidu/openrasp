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

#ifndef _OPENRASP_AGENT_MANAGER_H_
#define _OPENRASP_AGENT_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include "openrasp_ctrl_block.h"
#include "openrasp_agent.h"
#include "log_collect_item.h"
#include <fstream>
#include <sys/prctl.h>
#include <memory>

#if defined(__linux__)
#define AGENT_SET_PROC_NAME(name) prctl(PR_SET_NAME, (name), 0, 0, 0)
#else
#define AGENT_SET_PROC_NAME(name)
#endif
#define HEARTBEAT_AGENT_PR_NAME "rasp-heartbeat"
#define LOG_AGENT_PR_NAME "rasp-log"

namespace openrasp
{

class ShmManager;
class BaseAgent;
class HeartBeatAgent;
class LogAgent;
class OpenraspCtrlBlock;
class LogCollectItem;

class OpenraspAgentManager : public BaseManager
{
public:
  OpenraspAgentManager();
  bool startup();
  bool shutdown();
  bool verify_ini_correct();
  bool agent_remote_register();

  long get_plugin_update_timestamp();

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

  void set_plugin_md5(const char *plugin_md5);
  const char *get_plugin_md5();

private:
  bool create_share_memory();
  bool destroy_share_memory();

  bool process_agent_startup();
  void process_agent_shutdown();

  void supervisor_run();
  pid_t search_fpm_master_pid();
  void check_work_processes_survival();

private:
  OpenraspCtrlBlock *agent_ctrl_block;
  static const int task_interval = 300;
  char local_ip[64] = {0};
  pid_t init_process_pid;
  bool has_registered = false;
};

extern std::unique_ptr<OpenraspAgentManager> oam;

} // namespace openrasp

#endif
