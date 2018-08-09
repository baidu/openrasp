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

#include "mm/shm_manager.h"
#include "openrasp_ctrl_block.h"
#include "utils/curl_helper.h"

#define PLUGIN_AGENT_PR_NAME "plugin-agent"
#define LOG_AGENT_PR_NAME "log-agent"

namespace openrasp
{

class ShmManager;
class OpenraspCtrlBlock;

class LogDirInfo
{
public:
  const std::string dir_abs_path;
  const std::string backend_url;
  const std::string prefix;
  std::ifstream ifs;
  int fpos = 0;

  LogDirInfo(const std::string dir_abs_path, const std::string prefix, const std::string backend_url)
      : dir_abs_path(dir_abs_path), prefix(prefix), backend_url(backend_url)
  {
  }
};

class OpenraspAgentManager
{

public:
  OpenraspAgentManager(ShmManager *mm);

  bool startup();
  bool shutdown();

  long get_plugin_update_timestamp()
  {
    return (!initialized || nullptr == _agent_ctrl_block) ? 0 : _agent_ctrl_block->get_last_update_time();
  }

private:
  bool create_share_memory();
  bool destroy_share_memory();

  void supervisor_run();
  pid_t search_master_pid();
  bool process_agent_startup();
  void process_agent_shutdown();
  void install_signal_handler(__sighandler_t signal_handler);

  //for plugin update
  void plugin_agent_run();
  std::string clear_old_offcial_plugins();
  void update_local_offcial_plugin(std::string plugin_abs_path, const char *plugin, const char *version);

  //for log collect
  void log_agent_run();
  std::string get_formatted_date_suffix(long timestamp);
  void post_logs_via_curl(std::string log_arr, CURL *curl, std::string url_string);

private:
  ShmManager *_mm;
  pid_t first_process_pid;
  std::string _root_dir;
  std::string _backend;
  bool initialized = false;
  std::string _default_slash;
  OpenraspCtrlBlock *_agent_ctrl_block;
  static const int supervisor_interval = 10;
};

extern ShmManager sm;
extern OpenraspAgentManager oam;

} // namespace openrasp

#endif
