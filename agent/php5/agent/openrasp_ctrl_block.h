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

#ifndef _OPENRASP_CONTROL_BLOCK_
#define _OPENRASP_CONTROL_BLOCK_

#include <string>
namespace openrasp
{

class OpenraspCtrlBlock
{
  public:
    void set_supervisor_id(pid_t supervisor_id);
    pid_t get_supervisor_id();
    void set_plugin_agent_id(unsigned long plugin_agent_id);
    unsigned long get_plugin_agent_id();
    void set_log_agent_id(unsigned long log_agent_id);
    unsigned long get_log_agent_id();
    void set_plugin_md5(std::string plugin_md5);
    const char *get_plugin_md5();

  private:
    unsigned long _plugin_agent_id = 0;
    unsigned long _log_agent_id = 0;
    pid_t _supervisor_id;
    char _plugin_md5[33];
};

} // namespace openrasp

#endif
