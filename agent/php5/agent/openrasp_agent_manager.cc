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

#include "openrasp.h"
#include "openrasp_agent_manager.h"
#include "openrasp_agent_runner.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/digest.h"

namespace openrasp
{

typedef void (OpenraspCtrlBlock::*agent_id_setter_t)(unsigned long plugin_agent_id);

typedef struct agent_info_t
{
	const std::string name;
	bool is_alive;
	agent_id_setter_t agent_id_setter;
	BaseAgentRunner *baseAgentRunner;
	unsigned long agent_pid;
} agent_info;

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: _mm(mm), _agent_ctrl_block(NULL), _root_dir(openrasp_ini.root_dir)
{
}

int OpenraspAgentManager::startup()
{
	_create_share_memory();
	_write_local_plugin_md5_to_shm();
	_agent_startup();
	return SUCCESS;
}

int OpenraspAgentManager::_create_share_memory()
{
	_agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(
		_mm->create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock)));

	return SUCCESS;
}

int OpenraspAgentManager::shutdown()
{
	pid_t supervisor_id = static_cast<pid_t>(_agent_ctrl_block->get_supervisor_id());
	pid_t plugin_agent_id = _agent_ctrl_block->get_plugin_agent_id();
	pid_t log_agent_id = _agent_ctrl_block->get_log_agent_id();
	kill(supervisor_id, SIGTERM);
	kill(plugin_agent_id, SIGTERM);
	kill(log_agent_id, SIGTERM);
	_destroy_share_memory();
	return SUCCESS;
}

int OpenraspAgentManager::_destroy_share_memory()
{
	this->_mm->destroy(SHMEM_SEC_CTRL_BLOCK);
	return SUCCESS;
}

int OpenraspAgentManager::_agent_startup()
{
	_process_agent_startup();
	return SUCCESS;
}

int OpenraspAgentManager::_process_agent_startup()
{
	pid_t pid = fork();
	if (pid < 0)
	{
		return FAILURE;
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
		_agent_ctrl_block->set_supervisor_id(pid);
	}
	return SUCCESS;
}

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

static std::vector<agent_info> agent_list =
	{
		{"plugin-agent", false, &OpenraspCtrlBlock::set_plugin_agent_id, new PluginAgentRunner(), 0},
		{"log-agent", false, &OpenraspCtrlBlock::set_log_agent_id, new LogAgentRunner(), 0}};

static void supervisor_sigchld_handler(int signal_no)
{
	pid_t p;
	int status;
	while ((p = waitpid(-1, &status, WNOHANG)) > 0)
	{
		rasp_info("pid die", strlen("pid die") TSRMLS_CC);
		/* Handle the death of pid p */
		// TODO update agent_list
	}
}

int OpenraspAgentManager::_write_local_plugin_md5_to_shm()
{
	std::ifstream ifs(_root_dir + "plugins/offical.js");
	if (ifs.good())
	{
		std::stringstream buffer;
		buffer << ifs.rdbuf();
		std::string _local_plugin_md5 = md5sum(static_cast<const void *>(buffer.str().c_str()), buffer.str().size());
		_agent_ctrl_block->set_plugin_md5(_local_plugin_md5);
	}
}

void OpenraspAgentManager::supervisor_run()
{
	OPENRASP_SET_PROC_NAME("openrasp-agent.supervisor");

	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = supervisor_sigchld_handler;
	sigaction(SIGCHLD, &sa_usr, NULL);

	super_install_signal_handler();
	while (true)
	{
		for (agent_info &ai : agent_list)
		{
			if (!ai.is_alive)
			{
				pid_t pid = fork();
				if (pid < 0)
				{
					openrasp_error(E_ERROR, AGENT_ERROR, _("fork plugin agent fail!"));
				}
				else if (pid == 0)
				{
					(ai.baseAgentRunner)->run();
				}
				else
				{
					ai.is_alive = true;
					ai.agent_pid = pid;
					(_agent_ctrl_block->*(ai.agent_id_setter))(pid);
				}
			}
		}
		sleep(1);
	}
}

} // namespace openrasp
