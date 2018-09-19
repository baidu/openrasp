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
#include "safe_shutdown_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"

extern "C"
{
#include "php_streams.h"
#include "php_main.h"
}

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

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: BaseManager(mm),
	  agent_ctrl_block(nullptr)
{
}

bool OpenraspAgentManager::startup()
{
	if (check_sapi_need_alloc_shm())
	{
		if (!create_share_memory())
		{
			return false;
		}
		process_agent_startup();
		initialized = true;
	}
	return true;
}

bool OpenraspAgentManager::shutdown()
{
	if (initialized)
	{
		assert(ssdm != nullptr);
		if (!ssdm->is_master_current_process())
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
	TSRMLS_FETCH();
	if (openrasp_ini.remote_management_enable && check_sapi_need_alloc_shm())
	{
		if (nullptr == openrasp_ini.backend_url)
		{
			openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.backend_url is required when remote management is enabled."));
			return false;
		}
		if (nullptr == openrasp_ini.app_id)
		{
			openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_id is required when remote management is enabled."));
			return false;
		}
		else
		{
			zval *match_res = nullptr;
			MAKE_STD_ZVAL(match_res);
			char *regex = "/^[0-9a-fA-F]{32}$/";
			const int regex_len = strlen(regex);
			openrasp_pcre_match(regex, regex_len, openrasp_ini.app_id, strlen(openrasp_ini.app_id), match_res TSRMLS_CC);
			if (Z_TYPE_P(match_res) != IS_LONG || Z_LVAL_P(match_res) != 1)
			{
				openrasp_error(E_WARNING, CONFIG_ERROR, _("openrasp.app_id must have 32 characters"));
				zval_ptr_dtor(&match_res);
				return false;
			}
			zval_ptr_dtor(&match_res);
		}
	}
	return true;
}

bool OpenraspAgentManager::create_share_memory()
{
	char *shm_block = shm_manager->create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock));
	if (shm_block && (agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_block)))
	{
		return true;
	}
	return false;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	agent_ctrl_block = nullptr;
	this->shm_manager->destroy(SHMEM_SEC_CTRL_BLOCK);
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
	TSRMLS_FETCH();
	while (true)
	{
		for (int i = 0; i < supervisor_interval; ++i)
		{
			if (0 == i)
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
			sleep(1);
			assert(ssdm != nullptr);
			struct stat sb;
			if (VCWD_STAT(("/proc/" + std::to_string(ssdm->get_master_pid())).c_str(), &sb) == -1 &&
				errno == ENOENT)
			{
				process_agent_shutdown();
			}
		}
	}
}

} // namespace openrasp
