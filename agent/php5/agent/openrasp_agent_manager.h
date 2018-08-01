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

#define PLUGIN_AGENT_PR_NAME "plugin-agent"
#define LOG_AGENT_PR_NAME "log-agent"

namespace openrasp
{

class ShmManager;
class OpenraspCtrlBlock;

class OpenraspAgentManager
{
public:
  OpenraspAgentManager(ShmManager *mm);

  void supervisor_run();
  void plugin_agent_run();
  void log_agent_run();
  int startup();
  int shutdown();
  long get_plugin_update_timestamp()
  {
    return _agent_ctrl_block ? _agent_ctrl_block->get_last_update_time() : 0;
  }

private:
  int _create_share_memory();
  int _destroy_share_memory();

  int _agent_startup();
  int _process_agent_startup();

  int _write_local_plugin_md5_to_shm();

private:
  ShmManager *_mm;
  OpenraspCtrlBlock *_agent_ctrl_block;
  std::string _root_dir;
  std::string _backend;
  bool initialized = false;
};

extern ShmManager sm;
extern OpenraspAgentManager oam;

} // namespace openrasp

#endif
