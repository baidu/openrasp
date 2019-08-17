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

#include "utils/JsonReader.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <memory>

#include "openrasp_log.h"
#include "openrasp_agent_manager.h"
#include "shared_config_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/file.h"
#include "utils/net.h"
#include "utils/hostname.h"
#include "agent/utils/os.h"
#include "openrasp_utils.h"
#include "agent/webdir/webdir_agent.h"

#ifdef HAVE_LINE_COVERAGE
#define SIGNAL_KILL_AGENT SIGTERM
#else
#define SIGNAL_KILL_AGENT SIGKILL
#endif

namespace openrasp
{
std::unique_ptr<OpenraspAgentManager> oam = nullptr;

std::vector<std::unique_ptr<BaseAgent>> agents;
static const std::string register_url_path = "/v1/agent/rasp";

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
	: agent_ctrl_block(nullptr), rwlock(nullptr)
{
	meta_size = ROUNDUP(sizeof(pthread_rwlock_t), 1 << 3);
}

OpenraspAgentManager::~OpenraspAgentManager()
{
	if (rwlock != nullptr)
	{
		delete rwlock;
		rwlock = nullptr;
	}
}

bool OpenraspAgentManager::startup()
{
	init_process_pid = getpid();
	if (need_alloc_shm_current_sapi())
	{
		if (!create_share_memory())
		{
			return false;
		}
		set_master_pid(init_process_pid);
		set_dependency_interval(OpenraspCtrlBlock::default_dependency_interval);
		set_webdir_scan_interval(OpenraspCtrlBlock::default_webdir_scan_interval);
		set_scan_limit(OpenraspCtrlBlock::default_scan_limit);
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
				set_master_pid(fpm_master_pid);
			}
		}
		if (get_master_pid() && getpid() != get_master_pid())
		{
			return true;
		}
		process_agent_shutdown();
		destroy_share_memory();
		initialized = false;
	}
	return true;
}

bool OpenraspAgentManager::create_share_memory()
{
	size_t total_size = meta_size + sizeof(OpenraspCtrlBlock);
	char *shm_block = BaseManager::sm.create(SHMEM_SEC_CTRL_BLOCK, total_size);
	if (shm_block)
	{
		memset(shm_block, 0, total_size);
		rwlock = new ReadWriteLock((pthread_rwlock_t *)shm_block, LOCK_PROCESS);
		char *shm_ctrl_block = shm_block + meta_size;
		agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_ctrl_block);
		initialized = true;
		return true;
	}
	return false;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	if (rwlock != nullptr)
	{
		delete rwlock;
		rwlock = nullptr;
	}
	agent_ctrl_block = nullptr;
	BaseManager::sm.destroy(SHMEM_SEC_CONF_BLOCK);
	initialized = false;
	return true;
}

bool OpenraspAgentManager::process_agent_startup()
{
	agents.push_back(std::move((std::unique_ptr<BaseAgent>)new HeartBeatAgent()));
	agents.push_back(std::move((std::unique_ptr<BaseAgent>)new LogAgent()));
	agents.push_back(std::move((std::unique_ptr<BaseAgent>)new WebDirAgent()));
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
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		setsid();
		supervisor_run();
	}
	else
	{
		set_supervisor_id(pid);
	}
	return true;
}

void OpenraspAgentManager::process_agent_shutdown()
{
	agents.clear();
	pid_t log_agent_id = get_log_agent_id();
	if (log_agent_id > 0)
	{
		kill(log_agent_id, SIGNAL_KILL_AGENT);
	}
	pid_t webdir_agent_id = get_webdir_agent_id();
	if (webdir_agent_id > 0)
	{
		kill(webdir_agent_id, SIGNAL_KILL_AGENT);
	}
	pid_t plugin_agent_id = get_plugin_agent_id();
	if (plugin_agent_id > 0)
	{
		kill(plugin_agent_id, SIGNAL_KILL_AGENT);
	}
	pid_t supervisor_id = get_supervisor_id();
	if (supervisor_id > 0)
	{
		kill(supervisor_id, SIGNAL_KILL_AGENT);
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
		update_log_level();
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
			if (!pid_alive(std::to_string(get_master_pid())))
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

	std::string url_string = std::string(openrasp_ini.backend_url) + register_url_path;
	std::string php_version = get_phpversion();

	JsonReader json_reader;
	json_reader.write_string({"id"}, scm->get_rasp_id());
	json_reader.write_string({"hostname"}, get_hostname());
	json_reader.write_string({"language"}, "php");
	json_reader.write_string({"language_version"}, php_version);
	json_reader.write_string({"server_type"}, sapi_module.name);
	json_reader.write_string({"server_version"}, php_version);
	json_reader.write_string({"rasp_home"}, openrasp_ini.root_dir);
	json_reader.write_string({"register_ip"}, local_ip);
	json_reader.write_string({"version"}, PHP_OPENRASP_VERSION);
	json_reader.write_int64({"heartbeat_interval"}, openrasp_ini.heartbeat_interval);
	std::string cgroup_first_line = get_line_content("/proc/self/cgroup", 1);
	std::size_t found = cgroup_first_line.find(":/docker/");
	if (found != std::string::npos)
	{
		json_reader.write_string({"host_type"}, "docker");
	}
	std::map<std::string, std::string> env_map = get_env_map();
	json_reader.write_map({"environ"}, env_map);
	std::string json_content = json_reader.dump();

	BackendRequest backend_request(url_string, json_content.c_str());
	openrasp_error(LEVEL_DEBUG, REGISTER_ERROR, _("url:%s body:%s"), url_string.c_str(), json_content.c_str());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(LEVEL_WARNING, REGISTER_ERROR, _("CURL error: %s (%d), url: %s"),
					   backend_request.get_curl_err_msg(), backend_request.get_curl_code(),
					   url_string.c_str());
		return false;
	}
	openrasp_error(LEVEL_DEBUG, REGISTER_ERROR, _("%s"), res_info->to_string().c_str());
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
		if (file_exists(stat_file_path))
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

void OpenraspAgentManager::set_supervisor_id(pid_t supervisor_id)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_supervisor_id(supervisor_id);
	}
}

pid_t OpenraspAgentManager::get_supervisor_id()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_supervisor_id();
	}
	return 0;
}

void OpenraspAgentManager::set_plugin_agent_id(pid_t plugin_agent_id)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_plugin_agent_id(plugin_agent_id);
	}
}

pid_t OpenraspAgentManager::get_plugin_agent_id()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_plugin_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_webdir_agent_id(pid_t webdir_agent_id)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_webdir_agent_id(webdir_agent_id);
	}
}

pid_t OpenraspAgentManager::get_webdir_agent_id()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_webdir_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_log_agent_id(pid_t log_agent_id)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_log_agent_id(log_agent_id);
	}
}

pid_t OpenraspAgentManager::get_log_agent_id()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_log_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_master_pid(pid_t master_pid)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_master_pid(master_pid);
	}
}

pid_t OpenraspAgentManager::get_master_pid()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_master_pid();
	}
	return 0;
}

void OpenraspAgentManager::set_plugin_version(const char *plugin_version)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_plugin_version(plugin_version);
	}
}
const char *OpenraspAgentManager::get_plugin_version()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_plugin_version();
	}
	return "";
}

void OpenraspAgentManager::set_plugin_name(const char *plugin_name)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_plugin_name(plugin_name);
	}
}
const char *OpenraspAgentManager::get_plugin_name()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_plugin_name();
	}
	return "";
}

void OpenraspAgentManager::set_plugin_md5(const char *plugin_md5)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_plugin_md5(plugin_md5);
	}
}
const char *OpenraspAgentManager::get_plugin_md5()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_plugin_md5();
	}
	return "";
}

long OpenraspAgentManager::get_plugin_update_timestamp()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_last_update_time();
	}
	return 0;
}

bool OpenraspAgentManager::path_writable()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return strcmp(agent_ctrl_block->get_webroot_path(), "") == 0;
	}
	return false;
}
bool OpenraspAgentManager::path_exist(ulong hash)
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->webroot_found(hash);
	}
	return true;
}

void OpenraspAgentManager::write_webroot_path(const char *webroot_path)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_webroot_path(webroot_path);
	}
}

bool OpenraspAgentManager::consume_webroot_path(std::string &webroot_path)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		const char *path = agent_ctrl_block->get_webroot_path();
		if (strlen(path) > 0)
		{
			ulong webroot_hash = zend_inline_hash_func(path, strlen(path));
			int count = agent_ctrl_block->get_webroot_count();
			agent_ctrl_block->set_webroot_hash(count, webroot_hash);
			agent_ctrl_block->set_webroot_count(count + 1);
			webroot_path = std::string(path);
			agent_ctrl_block->set_webroot_path("");
			return true;
		}
	}
	return false;
}

int OpenraspAgentManager::get_dependency_interval()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_dependency_interval();
	}
	return OpenraspCtrlBlock::default_dependency_interval;
}

void OpenraspAgentManager::set_dependency_interval(int dependency_interval)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_dependency_interval(dependency_interval);
	}
}

int OpenraspAgentManager::get_webdir_scan_interval()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_webdir_scan_interval();
	}
	return OpenraspCtrlBlock::default_webdir_scan_interval;
}

void OpenraspAgentManager::set_webdir_scan_interval(int webdir_scan_interval)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_webdir_scan_interval(webdir_scan_interval);
	}
}

long OpenraspAgentManager::get_scan_limit()
{
	if (rwlock != nullptr && rwlock->read_try_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(rwlock);
		return agent_ctrl_block->get_scan_limit();
	}
	return OpenraspCtrlBlock::default_scan_limit;
}

void OpenraspAgentManager::set_scan_limit(long scan_limit)
{
	if (rwlock != nullptr && rwlock->write_try_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(rwlock);
		agent_ctrl_block->set_scan_limit(scan_limit);
	}
}

} // namespace openrasp
