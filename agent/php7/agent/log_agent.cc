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

#include "openrasp_agent.h"
#include "openrasp_hook.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "shared_config_manager.h"
#include "agent/utils/os.h"
#include "utils/file.h"

namespace openrasp
{

volatile int LogAgent::signal_received = 0;
const double LogAgent::factor = 2.0;
static const std::string LOG_AGENT_PR_NAME = "rasp-log";

LogAgent::LogAgent()
	: BaseAgent(LOG_AGENT_PR_NAME)
{
}

void LogAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->set_log_agent_id(agent_pid);
}

pid_t LogAgent::get_pid_from_shm()
{
	return oam->get_log_agent_id();
}

void LogAgent::run()
{
	pid_t supervisor_pid = getppid();
	AGENT_SET_PROC_NAME(this->name.c_str());

	install_sigterm_handler(
		[](int signal_no) {
			LogAgent::signal_received = signal_no;
		});

	LogCollectItem alarm_dir_info(ALARM_LOGGER, true);
	LogCollectItem policy_dir_info(POLICY_LOGGER, true);
	LogCollectItem plugin_dir_info(PLUGIN_LOGGER, false);
	LogCollectItem rasp_dir_info(RASP_LOGGER, true);
	std::vector<LogCollectItem *> log_dirs{&alarm_dir_info, &policy_dir_info, &plugin_dir_info, &rasp_dir_info};
	sleep(1);

	unsigned long current_interval = LogAgent::log_push_interval;
	while (true)
	{
		update_log_level();
		for (int i = 0; i < log_dirs.size(); ++i)
		{
			LogCollectItem *ldi = log_dirs[i];
			if (ldi->has_error())
			{
				continue;
			}
			bool file_rotate = ldi->need_rotate();
			if (ldi->get_collect_enable() &&
				file_exists(ldi->get_active_log_file()))
			{
				ldi->update_collect_status();
				ldi->refresh_cache_body();
				std::string post_body = ldi->get_cache_body();
				std::string url = ldi->get_cpmplete_url();
				if (!post_body.empty())
				{
					bool result = post_logs_via_curl(post_body, url);
					if (result)
					{
						ldi->clear_cache_body();
						ldi->update_status_snapshot();
					}
					current_interval =
						result
							? LogAgent::log_push_interval
							: increase_interval_by_factor(current_interval, LogAgent::factor, LogAgent::max_interval);
				}
			}
			ldi->handle_rotate(file_rotate);
		}

		for (long i = 0; i < current_interval; ++i)
		{
			sleep(1);
			if (!pid_alive(std::to_string(oam->get_master_pid())) ||
				!pid_alive(std::to_string(supervisor_pid)) ||
				getpid() != get_pid_from_shm() ||
				LogAgent::signal_received == SIGTERM)
			{
				exit(0);
			}
		}
	}
}

bool LogAgent::post_logs_via_curl(std::string &log_arr, std::string &url_string)
{
	BackendRequest backend_request;
	backend_request.set_url(url_string);
	backend_request.add_post_fields(log_arr);
	openrasp_error(LEVEL_DEBUG, LOGCOLLECT_ERROR, _("url:%s body:%s"), url_string.c_str(), log_arr.c_str());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(LEVEL_WARNING, LOGCOLLECT_ERROR, _("CURL error: %s (%d), url: %s"),
					   backend_request.get_curl_err_msg(), backend_request.get_curl_code(),
					   url_string.c_str());
		return false;
	}
	openrasp_error(LEVEL_DEBUG, LOGCOLLECT_ERROR, _("%s"), res_info->to_string().c_str());
	if (res_info->has_error())
	{
		openrasp_error(LEVEL_WARNING, LOGCOLLECT_ERROR, _("Fail to parse response body, error message %s."),
					   res_info->get_error_msg().c_str());
		return false;
	}
	if (!res_info->http_code_ok())
	{
		openrasp_error(LEVEL_WARNING, LOGCOLLECT_ERROR, _("Unexpected http response code: %ld, url: %s."),
					   res_info->get_http_code(), url_string.c_str());
		return false;
	}
	return true;
}
} // namespace openrasp
