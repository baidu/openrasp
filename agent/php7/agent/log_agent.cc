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
const double LogAgent::factor = 2.0;

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

	install_signal_handler(
		[](int signal_no) {
			LogAgent::signal_received = signal_no;
		});

	LogCollectItem alarm_dir_info(ALARM_LOG_DIR_NAME, "/v1/agent/log/attack", true);
	LogCollectItem policy_dir_info(POLICY_LOG_DIR_NAME, "/v1/agent/log/policy", true);
	LogCollectItem plugin_dir_info(PLUGIN_LOG_DIR_NAME, "/v1/agent/log/plugin", false);
	LogCollectItem rasp_dir_info(RASP_LOG_DIR_NAME, "/v1/agent/log/rasp", false);
	std::vector<LogCollectItem *> log_dirs{&alarm_dir_info, &policy_dir_info, &plugin_dir_info};

	long current_interval = LogAgent::log_push_interval;
	while (true)
	{
		LOG_G(rasp_logger).set_level(scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
		for (int i = 0; i < log_dirs.size(); ++i)
		{
			LogCollectItem *ldi = log_dirs[i];
			bool file_rotate = ldi->need_rotate();
			if (access(ldi->get_active_log_file().c_str(), F_OK) == 0)
			{
				ldi->determine_fpos();
				std::string post_body;
				std::string url = ldi->get_cpmplete_url();
				if (ldi->get_post_logs(post_body))
				{
					bool result = post_logs_via_curl(post_body, url);
					if (result)
					{
						ldi->update_status();
					}
					current_interval = result
										   ? LogAgent::log_push_interval
										   : increase_interval_by_factor(current_interval, LogAgent::factor, LogAgent::max_interval);
				}
			}
			ldi->handle_rotate(file_rotate);
		}

		for (long i = 0; i < current_interval; ++i)
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
		openrasp_error(E_WARNING, LOGCOLLECT_ERROR, _("CURL error code: %d, url: %s."),
					   backend_request.get_curl_code(), url_string.c_str());
		return false;
	}
	if (!res_info->http_code_ok())
	{
		openrasp_error(E_WARNING, LOGCOLLECT_ERROR, _("Unexpected http response code: %ld, url: %s."),
					   res_info->get_http_code(), url_string.c_str());
		return false;
	}
	return true;
}
} // namespace openrasp
