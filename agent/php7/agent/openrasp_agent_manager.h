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

#ifndef _OPENRASP_AGENT_MANAGER_H_
#define _OPENRASP_AGENT_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include "openrasp_ctrl_block.h"
#include "plugin_info_block.h"
#include "webdir/webdir_ctrl_block.h"
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
  virtual ~OpenraspAgentManager();
  bool startup();
  bool shutdown();

  /*agent block*/
  void agent_remote_register();
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

  /*plugin info*/
  long get_plugin_update_timestamp();
  void set_plugin_version(const char *plugin_version);
  const char *get_plugin_version();

  void set_plugin_name(const char *plugin_name);
  const char *get_plugin_name();

  void set_plugin_md5(const char *plugin_md5);
  const char *get_plugin_md5();

  /*webdir block*/
  void set_webdir_scan_regex(const char *webdir_scan_regex);
  const char *get_webdir_scan_regex();

  bool path_writable();
  bool path_exist(ulong hash);
  void write_webroot_path(const char *webroot_path);
  bool consume_webroot_path(std::string &webroot_path);

  int get_dependency_interval();
  void set_dependency_interval(int dependency_interval);

  int get_webdir_scan_interval();
  void set_webdir_scan_interval(int webdir_scan_interval);

  long get_scan_limit();
  void set_scan_limit(long scan_limit);

private:
  bool create_share_memory();
  bool destroy_share_memory();

  bool supervisor_startup();
  void supervisor_shutdown();

  void supervisor_run();
  pid_t search_fpm_master_pid();
  void ensure_agent_processes_survival();
  void kill_agent_processes();

private:
  int rwlock_size;
  static const int register_interval = 300;
  char local_ip[64] = {0};
  pid_t init_process_pid;

  ReadWriteLock *agent_rwlock = nullptr;
  OpenraspCtrlBlock *agent_ctrl_block = nullptr;

  ReadWriteLock *plugin_rwlock = nullptr;
  PluginInfoBlock *plugin_info_block = nullptr;

  ReadWriteLock *webdir_rwlock = nullptr;
  WebdirCtrlBlock *webdir_ctrl_block = nullptr;
};

extern std::unique_ptr<OpenraspAgentManager> oam;

} // namespace openrasp

#endif
