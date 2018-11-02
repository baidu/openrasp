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

#include "openrasp_config.h"
#include "openrasp_agent.h"
#include "openrasp_hook.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include "shared_config_manager.h"

namespace openrasp
{

volatile int LogAgent::signal_received = 0;

LogAgent::LogAgent()
	: BaseAgent(LOG_AGENT_PR_NAME)
{
}

void LogAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->agent_ctrl_block->set_log_agent_id(agent_pid);
}

void LogAgent::run()
{
	AGENT_SET_PROC_NAME(this->name.c_str());
	TSRMLS_FETCH();

	install_signal_handler(
		[](int signal_no) {
			LogAgent::signal_received = signal_no;
		});

	LogCollectItem alarm_dir_info(ALARM_LOG_DIR_NAME, "/v1/agent/log/attack", true TSRMLS_CC);
	LogCollectItem policy_dir_info(POLICY_LOG_DIR_NAME, "/v1/agent/log/policy", true TSRMLS_CC);
	LogCollectItem plugin_dir_info(PLUGIN_LOG_DIR_NAME, "/v1/agent/log/plugin", false TSRMLS_CC);
	std::vector<LogCollectItem *> log_dirs{&alarm_dir_info, &policy_dir_info, &plugin_dir_info};
	while (true)
	{
		for (int i = 0; i < log_dirs.size(); ++i)
		{
			LogCollectItem *ldi = log_dirs[i];
			bool file_rotate = ldi->need_rotate();
			if (file_exist(ldi->get_active_log_file().c_str() TSRMLS_CC))
			{
				ldi->determine_fpos(TSRMLS_C);
				std::string post_body;
				std::string url = ldi->get_cpmplete_url();
				if (ldi->get_post_logs(post_body) &&
					post_logs_via_curl(post_body, url))
				{
					ldi->update_status();
				}
			}
			ldi->handle_rotate(file_rotate TSRMLS_CC);
		}
		for (int i = 0; i < LogAgent::log_push_interval; ++i)
		{
			sleep(1);
			if (LogAgent::signal_received == SIGTERM)
			{
				exit(0);
			}
		}
	}
}

bool LogAgent::post_logs_via_curl(std::string &log_arr, std::string &url_string)
{
	BackendRequest backend_request(url_string, log_arr.c_str());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(E_WARNING, LOGCOLLECT_ERROR, _("CURL error code: %d."), backend_request.get_curl_code());
		return false;
	}
	return res_info->verify(LOGCOLLECT_ERROR);
}
} // namespace openrasp
