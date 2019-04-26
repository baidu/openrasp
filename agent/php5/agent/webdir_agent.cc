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

#include "openrasp_agent.h"
#include "agent/utils/os.h"

namespace openrasp
{

volatile int WebDirAgent::signal_received = 0;

WebDirAgent::WebDirAgent()
	: BaseAgent(WEBDIR_AGENT_PR_NAME)
{
}

void WebDirAgent::run()
{
	pid_t supervisor_pid = getppid();
	AGENT_SET_PROC_NAME(this->name.c_str());
	install_signal_handler(
		[](int signal_no) {
			WebDirAgent::signal_received = signal_no;
		});

	while (true)
	{
		update_log_level();
		collect_webroot_path();
		for (long i = 0; i < 10; ++i)
		{
			sleep(1);
			if (!pid_alive(std::to_string(oam->get_master_pid())) ||
				!pid_alive(std::to_string(supervisor_pid)) ||
				WebDirAgent::signal_received == SIGTERM)
			{
				exit(0);
			}
		}
	}
}

void WebDirAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->set_webdir_agent_id(agent_pid);
}

void WebDirAgent::collect_webroot_path()
{
	std::string webroot_path;
	if (oam->consume_webroot_path(webroot_path))
	{
		webroots.insert(webroot_path);
	}
}

} // namespace openrasp
