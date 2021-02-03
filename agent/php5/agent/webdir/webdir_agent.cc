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

#include "webdir_agent.h"
#include "webdir_utils.h"
#include "agent/utils/os.h"
#include "dependency_writer.h"
#include "agent/shared_config_manager.h"

namespace openrasp
{

static const std::string dependency_url_path = "/v1/agent/dependency";
static const std::string WEBDIR_AGENT_PR_NAME = "rasp-webdir";
volatile int WebDirAgent::signal_received = 0;

WebDirAgent::WebDirAgent()
	: BaseAgent(WEBDIR_AGENT_PR_NAME)
{
}

void WebDirAgent::run()
{
	pid_t supervisor_pid = getppid();
	AGENT_SET_PROC_NAME(this->name.c_str());
	install_sigterm_handler(
		[](int signal_no) {
			WebDirAgent::signal_received = signal_no;
		});
	long start = (long)time(nullptr);
	long last_dependency_check_time = start;
	long last_sensitive_file_scan_time = start;
	while (true)
	{
		update_log_level();
		sleep(1);
		if (!pid_alive(std::to_string(oam->get_master_pid())) ||
			!pid_alive(std::to_string(supervisor_pid)) ||
			WebDirAgent::signal_received == SIGTERM)
		{
			exit(0);
		}
		//skip while config has not been written into shm
		if (0 == scm->get_config_last_update())
		{
			continue;
		}
		bool force = false;
		if (collect_webroot_path())
		{
			force = true;
		}
		long now = (long)time(nullptr);
		if (force ||
			now <= last_sensitive_file_scan_time ||
			now - last_sensitive_file_scan_time >= oam->get_webdir_scan_interval())
		{
			sensitive_file_scan();
			last_sensitive_file_scan_time = now;
		}
		if (force ||
			now < last_dependency_check_time ||
			now - last_dependency_check_time >= oam->get_dependency_interval())
		{
			dependency_check();
			last_dependency_check_time = now;
		}
	}
}

void WebDirAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->set_webdir_agent_id(agent_pid);
}

pid_t WebDirAgent::get_pid_from_shm()
{
	return oam->get_webdir_agent_id();
}

bool WebDirAgent::collect_webroot_path()
{
	bool result = false;
	std::string webroot_path;
	if (oam->consume_webroot_path(webroot_path))
	{
		webdir_detector.insert_directory(webroot_path);
		result = true;
	}
	return result;
}

void WebDirAgent::sensitive_file_scan()
{
	std::map<std::string, std::vector<std::string>> sensitive_file_map = webdir_detector.sensitive_file_detect(oam->get_scan_limit());
	sensitive_files_policy_alarm(sensitive_file_map);
}

void WebDirAgent::dependency_check()
{
	if (webdir_detector.webdirs_composer_lock_modified())
	{
		std::vector<DependencyItem> deps = webdir_detector.dependency_detect();
		DependencyWriter dep_writer;
		dep_writer.write_string({"rasp_id"}, scm->get_rasp_id());
		dep_writer.write_dependencys({"dependency"}, deps, "composer");

		std::string url_string = std::string(openrasp_ini.backend_url) + dependency_url_path;
		std::string dep_body = dep_writer.dump();
		BackendRequest backend_request;
		backend_request.set_url(url_string);
		backend_request.add_post_fields(dep_body);
		openrasp_error(LEVEL_DEBUG, DEPENDENCY_ERROR, _("url:%s body:%s"), url_string.c_str(), dep_body.c_str());
		std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
		if (!res_info)
		{
			openrasp_error(LEVEL_WARNING, DEPENDENCY_ERROR, _("CURL error: %s (%d), url: %s"),
						   backend_request.get_curl_err_msg(), backend_request.get_curl_code(),
						   url_string.c_str());
		}
		else
		{
			openrasp_error(LEVEL_DEBUG, DEPENDENCY_ERROR, _("%s"), res_info->to_string().c_str());
			if (!res_info->http_code_ok())
			{
				openrasp_error(LEVEL_WARNING, DEPENDENCY_ERROR, _("Unexpected http response code: %ld, url: %s."),
							   res_info->get_http_code(), url_string.c_str());
			}
		}
	}
}

} // namespace openrasp
