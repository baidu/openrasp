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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <memory>

#include "openrasp_log.h"
#include "openrasp_shared_alloc.h"
#include "openrasp_agent_manager.h"
#include "shared_config_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/regex.h"
#include "utils/net.h"
#include "openrasp_utils.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace openrasp
{
std::unique_ptr<OpenraspAgentManager> oam = nullptr;

std::vector<std::unique_ptr<BaseAgent>> agents;

static void super_signal_handler(int signal_no)
{
	exit(0);
}

static void super_install_signal_handler()
{
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = super_signal_handler;
	sigaction(SIGTERM, &sa_usr, NULL);
}

static void supervisor_sigchld_handler(int signal_no)
{
	pid_t p;
	int status;
	while ((p = waitpid(-1, &status, WNOHANG)) > 0)
	{
		for (int i = 0; i < agents.size(); ++i)
		{
			std::unique_ptr<BaseAgent> &agent_ptr = agents[i];
			if (p == agent_ptr->agent_pid)
			{
				agent_ptr->is_alive = false;
			}
		}
	}
}

OpenraspAgentManager::OpenraspAgentManager()
	: agent_ctrl_block(nullptr)
{
}

bool OpenraspAgentManager::startup()
{
	init_process_pid = getpid();
	if (check_sapi_need_alloc_shm())
	{
		if (!create_share_memory())
		{
			return false;
		}
		agent_ctrl_block->set_master_pid(init_process_pid);
		process_agent_startup();
		initialized = true;
	}
	return true;
}

bool OpenraspAgentManager::shutdown()
{
	if (initialized)
	{
		if (strcmp(sapi_module.name, "fpm-fcgi") == 0)
		{
			pid_t fpm_master_pid = search_fpm_master_pid();
			if (fpm_master_pid)
			{
				agent_ctrl_block->set_master_pid(fpm_master_pid);
			}
		}
		if (agent_ctrl_block->get_master_pid() && getpid() != agent_ctrl_block->get_master_pid())
		{
			return true;
		}
		process_agent_shutdown();
		destroy_share_memory();
		initialized = false;
	}
	return true;
}

bool OpenraspAgentManager::verify_ini_correct()
{
	if (openrasp_ini.remote_management_enable && check_sapi_need_alloc_shm())
	{
		if (nullptr == openrasp_ini.backend_url || strcmp(openrasp_ini.backend_url, "") == 0)
		{
			openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.backend_url is required when remote management is enabled."));
			return false;
		}
		if (nullptr == openrasp_ini.app_id || strcmp(openrasp_ini.app_id, "") == 0)
		{
			openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_id is required when remote management is enabled."));
			return false;
		}
		else
		{
			if (!regex_match(openrasp_ini.app_id, "^[0-9a-fA-F]{40}$"))
			{
				openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_id must be exactly 40 characters long."));
				return false;
			}
		}
		if (nullptr == openrasp_ini.app_secret || strcmp(openrasp_ini.app_secret, "") == 0)
		{
			openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_secret is required when remote management is enabled."));
			return false;
		}
		else
		{
			if (!regex_match(openrasp_ini.app_secret, "^[0-9a-zA-Z_-]{43,45}"))
			{
				openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_secret configuration format is incorrect."));
				return false;
			}
		}
	}
	return true;
}

bool OpenraspAgentManager::create_share_memory()
{
	char *shm_block = BaseManager::sm.create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock));
	if (shm_block && (agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_block)))
	{
		return true;
	}
	return false;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	agent_ctrl_block = nullptr;
	BaseManager::sm.destroy(SHMEM_SEC_CTRL_BLOCK);
	return true;
}

bool OpenraspAgentManager::process_agent_startup()
{
	if (openrasp_ini.plugin_update_enable)
	{
		agents.push_back(std::move((std::unique_ptr<BaseAgent>)new HeartBeatAgent()));
	}
	agents.push_back(std::move((std::unique_ptr<BaseAgent>)new LogAgent()));
	pid_t pid = fork();
	if (pid < 0)
	{
		return false;
	}
	else if (pid == 0)
	{
		setsid();
		supervisor_run();
	}
	else
	{
		agent_ctrl_block->set_supervisor_id(pid);
	}
	return true;
}

void OpenraspAgentManager::process_agent_shutdown()
{
	agents.clear();
	pid_t log_agent_id = agent_ctrl_block->get_log_agent_id();
	if (log_agent_id > 0)
	{
		kill(log_agent_id, SIGKILL);
	}
	if (openrasp_ini.plugin_update_enable)
	{
		pid_t plugin_agent_id = agent_ctrl_block->get_plugin_agent_id();
		if (plugin_agent_id > 0)
		{
			kill(plugin_agent_id, SIGKILL);
		}
	}
	pid_t supervisor_id = agent_ctrl_block->get_supervisor_id();
	if (supervisor_id > 0)
	{
		kill(supervisor_id, SIGKILL);
	}
}

void OpenraspAgentManager::supervisor_run()
{
	AGENT_SET_PROC_NAME("rasp-supervisor");
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = supervisor_sigchld_handler;
	sigaction(SIGCHLD, &sa_usr, NULL);

	super_install_signal_handler();
	while (true)
	{
		LOG_G(rasp_logger).set_level(scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
		for (int i = 0; i < task_interval; ++i)
		{
			if (i % task_interval == 0 && !has_registered)
			{
				has_registered = agent_remote_register();
			}
			if (i % 10 == 0 && has_registered)
			{
				check_work_processes_survival();
			}
			sleep(1);
			struct stat sb;
			if (stat(("/proc/" + std::to_string(agent_ctrl_block->get_master_pid())).c_str(), &sb) == -1 &&
				errno == ENOENT)
			{
				process_agent_shutdown();
			}
		}
	}
}

void OpenraspAgentManager::check_work_processes_survival()
{
	for (int i = 0; i < agents.size(); ++i)
	{
		std::unique_ptr<BaseAgent> &agent_ptr = agents[i];
		if (!agent_ptr->is_alive)
		{
			pid_t pid = fork();
			if (pid == 0)
			{
				agent_ptr->run();
			}
			else if (pid > 0)
			{
				agent_ptr->is_alive = true;
				agent_ptr->agent_pid = pid;
				agent_ptr->write_pid_to_shm(pid);
			}
		}
	}
}

bool OpenraspAgentManager::agent_remote_register()
{
	if (!fetch_source_in_ip_packets(local_ip, sizeof(local_ip), openrasp_ini.backend_url))
	{
		local_ip[0] = 0;
	}

	std::string url_string = std::string(openrasp_ini.backend_url) + "/v1/agent/rasp";

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("id");
	writer.String(scm->get_rasp_id().c_str());
	writer.Key("hostname");
	writer.String(scm->get_hostname().c_str());
	writer.Key("language");
	writer.String("php");
	writer.Key("language_version");
	writer.String(OPENRASP_PHP_VERSION);
	writer.Key("server_type");
	writer.String(sapi_module.name);
	writer.Key("server_version");
	writer.String(OPENRASP_PHP_VERSION);
	writer.Key("rasp_home");
	writer.String(openrasp_ini.root_dir);
	writer.Key("register_ip");
	writer.String(local_ip);
	writer.Key("version");
	writer.String(PHP_OPENRASP_VERSION);
	writer.Key("heartbeat_interval");
	writer.Int64(openrasp_ini.heartbeat_interval);
	writer.EndObject();

	BackendRequest backend_request(url_string, s.GetString());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(E_WARNING, REGISTER_ERROR, _("CURL error code: %d, url: %s."),
					   backend_request.get_curl_code(), url_string.c_str());
		return false;
	}
	return res_info->verify(REGISTER_ERROR);
}

pid_t OpenraspAgentManager::search_fpm_master_pid()
{
	std::vector<std::string> processes;
	openrasp_scandir("/proc", processes,
					 [](const char *filename) {
						 struct stat sb;
						 if (stat(("/proc/" + std::string(filename)).c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
						 {
							 return true;
						 }
						 return false;
					 });

	for (std::string pid : processes)
	{
		std::string stat_file_path = "/proc/" + pid + "/stat";
		if (access(stat_file_path.c_str(), F_OK) == 0)
		{
			std::ifstream ifs_stat(stat_file_path);
			std::string stat_line;
			std::getline(ifs_stat, stat_line);
			if (stat_line.empty())
			{
				continue;
			}
			std::stringstream stat_items(stat_line);
			std::string item;
			for (int i = 0; i < 4; ++i)
			{
				std::getline(stat_items, item, ' ');
			}
			int ppid = std::atoi(item.c_str());
			if (ppid == init_process_pid)
			{
				std::string cmdline_file_path = "/proc/" + pid + "/cmdline";
				std::ifstream ifs_cmdline(cmdline_file_path);
				std::string cmdline;
				std::getline(ifs_cmdline, cmdline);
				if (cmdline.find("php-fpm: master process") == 0)
				{
					return std::atoi(pid.c_str());
				}
			}
		}
	}
	return 0;
}

} // namespace openrasp
