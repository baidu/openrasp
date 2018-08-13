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
#include <signal.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <memory>

#include "openrasp.h"
#include "openrasp_log.h"
#include "openrasp_agent_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"

extern "C"
{
#include "openrasp_shared_alloc.h"
#include "php_streams.h"
#include "php_main.h"
}

namespace openrasp
{

ShmManager sm;
OpenraspAgentManager oam(&sm);

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
	: _mm(mm),
	  agent_ctrl_block(nullptr)
{
}

bool OpenraspAgentManager::startup()
{
	first_process_pid = getpid();
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
		if (strcmp(sapi_module.name, "fpm-fcgi") == 0)
		{
			pid_t master_pid = search_master_pid();
			agent_ctrl_block->set_master_pid(master_pid);
			if (master_pid && getpid() != master_pid)
			{
				return true;
			}
		}
		process_agent_shutdown();
		destroy_share_memory();
		initialized = false;
	}
	return true;
}

bool OpenraspAgentManager::create_share_memory()
{
	char *shm_block = _mm->create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock));
	if (shm_block && (agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_block)))
	{
		return true;
	}
	return false;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	agent_ctrl_block = nullptr;
	this->_mm->destroy(SHMEM_SEC_CTRL_BLOCK);
	return true;
}

pid_t OpenraspAgentManager::search_master_pid()
{
	TSRMLS_FETCH();
	std::vector<std::string> processes;
	openrasp_scandir("/proc", processes,
					 [](const char *filename) {
						 TSRMLS_FETCH();
						 struct stat sb;
						 if (VCWD_STAT(("/proc/" + std::string(filename)).c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
						 {
							 return true;
						 }
						 return false;
					 });
	for (std::string pid : processes)
	{
		std::string stat_file_path = "/proc/" + pid + "/stat";
		if (VCWD_ACCESS(stat_file_path.c_str(), F_OK) == 0)
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
			if (ppid == first_process_pid)
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

bool OpenraspAgentManager::process_agent_startup()
{
	if (openrasp_ini.plugin_update_enable)
	{
		agents.push_back(std::move((std::unique_ptr<BaseAgent>)new PluginAgent()));
	}
	agents.push_back(std::move((std::unique_ptr<BaseAgent>)new LogAgent()));
	agent_ctrl_block->set_master_pid(first_process_pid);
	pid_t pid = fork();
	if (pid < 0)
	{
		return false;
	}
	else if (pid == 0)
	{
		int fd;
		if (-1 != (fd = open("/dev/null", O_RDONLY)))
		{
			close(STDIN_FILENO);
			close(STDERR_FILENO);
			close(STDOUT_FILENO);
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDERR_FILENO);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
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
	pid_t supervisor_id = agent_ctrl_block->get_supervisor_id();
	pid_t plugin_agent_id = agent_ctrl_block->get_plugin_agent_id();
	pid_t log_agent_id = agent_ctrl_block->get_log_agent_id();
	kill(plugin_agent_id, SIGKILL);
	kill(log_agent_id, SIGKILL);
	kill(supervisor_id, SIGKILL);
}

void OpenraspAgentManager::supervisor_run()
{
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
			struct stat sb;
			if (VCWD_STAT(("/proc/" + std::to_string(agent_ctrl_block->get_master_pid())).c_str(), &sb) == -1 &&
				errno == ENOENT)
			{
				process_agent_shutdown();
			}
		}
	}
}

} // namespace openrasp
