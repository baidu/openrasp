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
#include "openrasp_log_collector.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/digest.h"
#include <curl/curl.h>

extern "C"
{
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
}

namespace openrasp
{

typedef void (OpenraspCtrlBlock::*agent_id_setter_t)(unsigned long plugin_agent_id);

typedef struct agent_info_t
{
	const std::string name;
	bool is_alive;
	agent_id_setter_t agent_id_setter;
	unsigned long agent_pid;
} agent_info;

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: _mm(mm),
	  _agent_ctrl_block(NULL),
	  _root_dir(openrasp_ini.root_dir),
	  _backend(openrasp_ini.backend)
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
		{PLUGIN_AGENT_PR_NAME, false, &OpenraspCtrlBlock::set_plugin_agent_id, 0},
		{LOG_AGENT_PR_NAME, false, &OpenraspCtrlBlock::set_log_agent_id, 0}};

static void supervisor_sigchld_handler(int signal_no)
{
	pid_t p;
	int status;
	while ((p = waitpid(-1, &status, WNOHANG)) > 0)
	{
		/* Handle the death of pid p */
		// TODO update agent_list
	}
}

volatile static int signal_received = 0;
static void signal_handler(int signal_no)
{
	signal_received = signal_no;
}

static void install_signal_handler()
{
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa_usr, NULL);
}

static void agent_exit()
{
	exit(0);
}

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
	data->append((char *)ptr, size * nmemb);
	return size * nmemb;
}

int OpenraspAgentManager::_write_local_plugin_md5_to_shm()
{
	std::ifstream ifs(_root_dir + "/plugins/offical.js");
	if (ifs.good())
	{
		std::stringstream buffer;
		buffer << ifs.rdbuf();
		std::string _local_plugin_md5 = md5sum(static_cast<const void *>(buffer.str().c_str()), buffer.str().size());
		_agent_ctrl_block->set_plugin_md5(_local_plugin_md5.c_str());
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
					OPENRASP_SET_PROC_NAME(ai.name.c_str());
					if (ai.name == PLUGIN_AGENT_PR_NAME)
					{
						plugin_agent_run();
					}
					else if (ai.name == LOG_AGENT_PR_NAME)
					{
						log_agent_run();
					}
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

void OpenraspAgentManager::plugin_agent_run()
{
	install_signal_handler();
	TSRMLS_FETCH();
	while (true)
	{
		auto curl = curl_easy_init();
		CURLcode res;
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, (_backend + "/test.php").c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

			std::string response_string;
			std::string header_string;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

			char *url;
			long response_code;
			double elapsed;

			res = curl_easy_perform(curl);
			if (CURLE_OK == res)
			{
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
				curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
				curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
				zval *return_value = nullptr;
				MAKE_STD_ZVAL(return_value);
				php_json_decode(return_value, (char *)response_string.c_str(), response_string.size(), 1, 512 TSRMLS_CC);
				zval **origin_zv;
				if (response_code >= 200 && response_code < 300)
				{
					if (Z_TYPE_P(return_value) == IS_ARRAY)
					{
						if (zend_hash_find(Z_ARRVAL_P(return_value), "content", strlen("content") + 1, (void **)&origin_zv) == SUCCESS &&
							Z_TYPE_PP(origin_zv) == IS_STRING)
						{
							std::ofstream out_file(_root_dir + "/plugins/offical.js", std::ofstream::in | std::ofstream::out | std::ofstream::trunc);
							if (out_file.good())
							{
								out_file << Z_STRVAL_PP(origin_zv);
								out_file.close();
							}
						}
						if (zend_hash_find(Z_ARRVAL_P(return_value), "md5", strlen("md5") + 1, (void **)&origin_zv) == SUCCESS &&
							Z_TYPE_PP(origin_zv) == IS_STRING)
						{
							_agent_ctrl_block->set_plugin_md5(Z_STRVAL_PP(origin_zv));
						}
					}
				}
				else
				{
					if (Z_TYPE_P(return_value) == IS_ARRAY &&
						zend_hash_find(Z_ARRVAL_P(return_value), "error", strlen("error") + 1, (void **)&origin_zv) == SUCCESS &&
						Z_TYPE_PP(origin_zv) == IS_STRING)
					{
						openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update offcial plugin, error: %s."), Z_STRVAL_PP(origin_zv));
					}
				}
				zval_ptr_dtor(&return_value);
			}
			curl_easy_cleanup(curl);
			curl = nullptr;
		}
		for (int i = 0; i < openrasp_ini.plugin_update_interval; ++i)
		{
			sleep(1);
			if (signal_received == SIGTERM)
			{
				agent_exit();
			}
		}
	}
}

void OpenraspAgentManager::log_agent_run()
{
	LogCollector logCollector;
	logCollector.run();
}

} // namespace openrasp
