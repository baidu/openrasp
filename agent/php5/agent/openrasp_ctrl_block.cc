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

extern "C"
{
    #include <stdio.h>
}
#include <string.h>
#include <unistd.h>
#include "openrasp_ctrl_block.h"

namespace openrasp
{

void OpenraspCtrlBlock::set_supervisor_id(pid_t supervisor_id)
{
    _supervisor_id = supervisor_id;
}

pid_t OpenraspCtrlBlock::get_supervisor_id()
{
    return _supervisor_id;
}

void OpenraspCtrlBlock::set_plugin_agent_id(unsigned long plugin_agent_id)
{
    _plugin_agent_id = plugin_agent_id;
}

unsigned long OpenraspCtrlBlock::get_plugin_agent_id()
{
    return _plugin_agent_id;
}

void OpenraspCtrlBlock::set_log_agent_id(unsigned long log_agent_id)
{
    _log_agent_id = log_agent_id;
}

unsigned long OpenraspCtrlBlock::get_log_agent_id()
{
    return _log_agent_id;
}

void OpenraspCtrlBlock::set_plugin_md5(std::string plugin_md5)
{
    strncpy(_plugin_md5, plugin_md5.c_str(), 32);
}

const char *OpenraspCtrlBlock::get_plugin_md5()
{
    return _plugin_md5;
}

} // namespace openrasp
