/*
 * Copyright 2017-2018 Baidu Inc.
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

class LogDirInfo
{
public:
  const std::string dir_abs_path;
  const std::string backend_url;
  const std::string prefix;
  std::ifstream ifs;
  int fpos = 0;
  long st_ino = 0;

  LogDirInfo(const std::string dir_abs_path, const std::string prefix, const std::string backend_url)
      : dir_abs_path(dir_abs_path), prefix(prefix), backend_url(backend_url)
  {
  }
};

class OpenraspAgentManager : public BaseManager
{

public:
  OpenraspCtrlBlock *agent_ctrl_block;

public:
  OpenraspAgentManager(ShmManager *mm);
  bool startup();
  bool shutdown();
  bool verify_ini_correct();
  std::string get_rasp_id();
  bool agent_remote_register();
  char *get_local_ip();

  long get_plugin_update_timestamp()
  {
    return (!initialized || nullptr == agent_ctrl_block) ? 0 : agent_ctrl_block->get_last_update_time();
  }

private:
  bool create_share_memory();
  bool destroy_share_memory();
  void supervisor_run();
  bool process_agent_startup();
  void process_agent_shutdown();
  bool calculate_rasp_id();
  pid_t search_fpm_master_pid();
  void check_work_processes_survival();

private:
  static const int task_interval = 300;
  char local_ip[64] = {0};
  pid_t init_process_pid;
  std::string rasp_id;
  bool has_registered = false;
};

extern std::unique_ptr<OpenraspAgentManager> oam;

} // namespace openrasp

#endif
