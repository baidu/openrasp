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

#include "utils/json_reader.h"
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
#include "utils/signal_interceptor.h"

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

static void suprevisor_sigterm_handler(int signal_no)
{
	openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Supervisor process receive signal SIGTERM"));
	exit(0);
}

static void supervisor_sigchld_handler(int signal_no)
{
	pid_t p;
	int status = 0;
	while ((p = waitpid(-1, &status, WNOHANG)) > 0)
	{
		for (int i = 0; i < agents.size(); ++i)
		{
			std::unique_ptr<BaseAgent> &agent_ptr = agents[i];
			if (p == agent_ptr->get_pid_from_shm())
			{
				openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("%s exited unexpectedly"), agent_ptr->get_name().c_str());
				agent_ptr->write_pid_to_shm(0);
			}
		}
	}
}

static void super_install_signal_handler()
{
	struct sigaction sa_usr_chld = {0};
	sa_usr_chld.sa_flags = 0;
	sa_usr_chld.sa_handler = supervisor_sigchld_handler;
	sigaction(SIGCHLD, &sa_usr_chld, NULL);

	struct sigaction sa_usr_term = {0};
	sa_usr_term.sa_flags = 0;
	sa_usr_term.sa_handler = suprevisor_sigterm_handler;
	sigaction(SIGTERM, &sa_usr_term, NULL);
}

OpenraspAgentManager::OpenraspAgentManager()
{
	agent_ctrl_block = nullptr;
	agent_rwlock = nullptr;

	plugin_info_block = nullptr;
	plugin_rwlock = nullptr;

	webdir_ctrl_block = nullptr;
	webdir_rwlock = nullptr;

	rwlock_size = ROUNDUP(sizeof(pthread_rwlock_t), 1 << 3);
}

OpenraspAgentManager::~OpenraspAgentManager()
{
	if (agent_rwlock != nullptr)
	{
		delete agent_rwlock;
		agent_rwlock = nullptr;
	}
	if (plugin_rwlock != nullptr)
	{
		delete plugin_rwlock;
		plugin_rwlock = nullptr;
	}
	if (webdir_rwlock != nullptr)
	{
		delete webdir_rwlock;
		webdir_rwlock = nullptr;
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
		set_dependency_interval(WebdirCtrlBlock::default_dependency_interval);
		set_webdir_scan_interval(WebdirCtrlBlock::default_webdir_scan_interval);
		set_scan_limit(WebdirCtrlBlock::default_scan_limit);
		supervisor_startup();
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
		supervisor_shutdown();
		destroy_share_memory();
		initialized = false;
	}
	return true;
}

bool OpenraspAgentManager::create_share_memory()
{
	/*agent block*/
	size_t agent_block_size = rwlock_size + sizeof(OpenraspCtrlBlock);
	char *agent_shm_block = BaseManager::sm.create(SHMEM_SEC_CTRL_BLOCK, agent_block_size);
	if (nullptr == agent_shm_block)
	{
		return false;
	}
	memset(agent_shm_block, 0, agent_block_size);
	agent_rwlock = new ReadWriteLock((pthread_rwlock_t *)agent_shm_block, LOCK_PROCESS);
	char *real_agent_ctrl_block = agent_shm_block + rwlock_size;
	agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(real_agent_ctrl_block);

	/*plugin info*/
	size_t plugin_info_size = rwlock_size + sizeof(PluginInfoBlock);
	char *plugin_shm_block = BaseManager::sm.create(SHMEM_SEC_PLUGIN_BLOCK, plugin_info_size);
	if (nullptr == plugin_shm_block)
	{
		return false;
	}
	memset(plugin_shm_block, 0, plugin_info_size);
	plugin_rwlock = new ReadWriteLock((pthread_rwlock_t *)plugin_shm_block, LOCK_PROCESS);
	char *real_plugin_info_block = plugin_shm_block + rwlock_size;
	plugin_info_block = reinterpret_cast<PluginInfoBlock *>(real_plugin_info_block);

	/*webdir block*/
	size_t webdir_block_size = rwlock_size + sizeof(WebdirCtrlBlock);
	char *webdir_shm_block = BaseManager::sm.create(SHMEM_SEC_WEBDIR_BLOCK, webdir_block_size);
	if (nullptr == webdir_shm_block)
	{
		return false;
	}
	memset(webdir_shm_block, 0, webdir_block_size);
	webdir_rwlock = new ReadWriteLock((pthread_rwlock_t *)webdir_shm_block, LOCK_PROCESS);
	char *real_webdir_ctrl_block = webdir_shm_block + rwlock_size;
	webdir_ctrl_block = reinterpret_cast<WebdirCtrlBlock *>(real_webdir_ctrl_block);

	initialized = true;
	return true;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	if (agent_rwlock != nullptr)
	{
		delete agent_rwlock;
		agent_rwlock = nullptr;
	}
	if (plugin_rwlock != nullptr)
	{
		delete plugin_rwlock;
		plugin_rwlock = nullptr;
	}
	if (webdir_rwlock != nullptr)
	{
		delete webdir_rwlock;
		webdir_rwlock = nullptr;
	}
	agent_ctrl_block = nullptr;
	BaseManager::sm.destroy(SHMEM_SEC_CONF_BLOCK);

	plugin_info_block = nullptr;
	BaseManager::sm.destroy(SHMEM_SEC_PLUGIN_BLOCK);

	webdir_ctrl_block = nullptr;
	BaseManager::sm.destroy(SHMEM_SEC_WEBDIR_BLOCK);

	initialized = false;
	return true;
}

bool OpenraspAgentManager::supervisor_startup()
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
		int fd = 0;
		if (-1 != (fd = open("/dev/null", O_RDWR)))
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
		openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("supervisor process starts, the pid is %d"), getpid());
		supervisor_run();
	}
	else
	{
		set_supervisor_id(pid);
	}
	return true;
}

void OpenraspAgentManager::supervisor_shutdown()
{
	kill_agent_processes();
	agents.clear();
	pid_t supervisor_id = get_supervisor_id();
	if (supervisor_id > 0)
	{
		kill(supervisor_id, SIGNAL_KILL_AGENT);
	}
}

void OpenraspAgentManager::supervisor_run()
{
	AGENT_SET_PROC_NAME("rasp-supervisor");
	super_install_signal_handler();
	general_signal_hook();

	int second_remain_to_register = 0;
	int second_remain_to_check_work_process = 0;
	while (true)
	{
		update_log_level();
		if (!get_registered())
		{
			if ((--second_remain_to_register) <= 0)
			{
				agent_remote_register();
				second_remain_to_register = register_interval;
			}
		}
		else
		{
			if ((--second_remain_to_check_work_process) <= 0)
			{
				ensure_agent_processes_survival();
				second_remain_to_check_work_process = 10;
			}
		}
		sleep(1);
		pid_t pid_before_detect = get_master_pid();
		if (!pid_alive(std::to_string(pid_before_detect)))
		{
			if (get_master_pid() == pid_before_detect)
			{
				openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("supervisor process is going to exit, cuz of master process (%d) is not alive"), pid_before_detect);
				supervisor_shutdown();
			}
			else
			{
				openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("master process pid changes from %d to %d"), pid_before_detect, get_master_pid());
			}
		}
	}
}

void OpenraspAgentManager::ensure_agent_processes_survival()
{
	for (int i = 0; i < agents.size(); ++i)
	{
		std::unique_ptr<BaseAgent> &agent_ptr = agents[i];
		if (0 == agent_ptr->get_pid_from_shm())
		{
			pid_t pid = fork();
			if (pid == 0)
			{
				openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("start %s process, the pid is %d"), agent_ptr->get_name().c_str(), getpid());
				agent_ptr->run();
			}
			else if (pid > 0)
			{
				agent_ptr->write_pid_to_shm(pid);
			}
		}
	}
}

void OpenraspAgentManager::kill_agent_processes()
{
	for (int i = 0; i < agents.size(); ++i)
	{
		std::unique_ptr<BaseAgent> &agent_ptr = agents[i];
		pid_t pid = agent_ptr->get_pid_from_shm();
		if (pid > 0)
		{
			kill(pid, SIGNAL_KILL_AGENT);
		}
	}
}

void OpenraspAgentManager::agent_remote_register()
{
	openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("agent starts remote register"));
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
	json_reader.write_string({"os"}, PHP_OS);
	json_reader.write_string({"language_version"}, php_version);
	json_reader.write_string({"server_type"}, sapi_module.name);
	json_reader.write_string({"server_version"}, php_version);
	json_reader.write_string({"rasp_home"}, openrasp_ini.root_dir);
	json_reader.write_string({"register_ip"}, local_ip);
	json_reader.write_string({"version"}, OpenRASPInfo::PHP_OPENRASP_VERSION);
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

	BackendRequest backend_request;
	backend_request.set_url(url_string);
	backend_request.add_post_fields(json_content);
	openrasp_error(LEVEL_DEBUG, REGISTER_ERROR, _("url:%s body:%s"), url_string.c_str(), json_content.c_str());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(LEVEL_WARNING, REGISTER_ERROR, _("CURL error: %s (%d), url: %s"),
					   backend_request.get_curl_err_msg(), backend_request.get_curl_code(),
					   url_string.c_str());
		return;
	}
	openrasp_error(LEVEL_DEBUG, REGISTER_ERROR, _("%s"), res_info->to_string().c_str());
	if (res_info->has_error())
	{
		openrasp_error(LEVEL_WARNING, REGISTER_ERROR, _("Fail to parse response body, error message %s."),
					   res_info->get_error_msg().c_str());
		return;
	}
	if (!res_info->http_code_ok())
	{
		openrasp_error(LEVEL_WARNING, REGISTER_ERROR, _("Unexpected http response code: %ld."),
					   res_info->get_http_code());
		return;
	}
	BaseReader *body_reader = res_info->get_body_reader();
	if (nullptr != body_reader)
	{
		int64_t status = body_reader->fetch_int64({"status"}, BackendResponse::default_int64);
		std::string description = body_reader->fetch_string({"description"}, "");
		if (0 != status)
		{
			openrasp_error(LEVEL_WARNING, REGISTER_ERROR, _("API error: %ld, description: %s"),
						   status, description.c_str());
			return;
		}
		else
		{
			set_registered(true);
		}
	}
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

/*agent block*/
void OpenraspAgentManager::set_supervisor_id(pid_t supervisor_id)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_supervisor_id(supervisor_id);
	}
}

pid_t OpenraspAgentManager::get_supervisor_id()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_supervisor_id();
	}
	return 0;
}

void OpenraspAgentManager::set_plugin_agent_id(pid_t plugin_agent_id)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_plugin_agent_id(plugin_agent_id);
	}
}

pid_t OpenraspAgentManager::get_plugin_agent_id()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_plugin_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_webdir_agent_id(pid_t webdir_agent_id)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_webdir_agent_id(webdir_agent_id);
	}
}

pid_t OpenraspAgentManager::get_webdir_agent_id()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_webdir_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_log_agent_id(pid_t log_agent_id)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_log_agent_id(log_agent_id);
	}
}

pid_t OpenraspAgentManager::get_log_agent_id()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_log_agent_id();
	}
	return 0;
}

void OpenraspAgentManager::set_master_pid(pid_t master_pid)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_master_pid(master_pid);
	}
}

pid_t OpenraspAgentManager::get_master_pid()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_master_pid();
	}
	return 0;
}

void OpenraspAgentManager::set_registered(bool registered)
{
	if (agent_rwlock != nullptr && agent_rwlock->write_lock() && agent_ctrl_block)
	{
		WriteUnLocker auto_unlocker(agent_rwlock);
		agent_ctrl_block->set_registered(registered);
	}
}

bool OpenraspAgentManager::get_registered()
{
	if (agent_rwlock != nullptr && agent_rwlock->read_lock() && agent_ctrl_block)
	{
		ReadUnLocker auto_unlocker(agent_rwlock);
		return agent_ctrl_block->get_registered();
	}
	return false;
}

/*plugin info*/
void OpenraspAgentManager::set_plugin_version(const char *plugin_version)
{
	if (plugin_rwlock != nullptr && plugin_rwlock->write_lock() && plugin_info_block)
	{
		WriteUnLocker auto_unlocker(plugin_rwlock);
		plugin_info_block->set_plugin_version(plugin_version);
	}
}
const char *OpenraspAgentManager::get_plugin_version()
{
	if (plugin_rwlock != nullptr && plugin_rwlock->read_lock() && plugin_info_block)
	{
		ReadUnLocker auto_unlocker(plugin_rwlock);
		return plugin_info_block->get_plugin_version();
	}
	return "";
}

void OpenraspAgentManager::set_plugin_name(const char *plugin_name)
{
	if (plugin_rwlock != nullptr && plugin_rwlock->write_lock() && plugin_info_block)
	{
		WriteUnLocker auto_unlocker(plugin_rwlock);
		plugin_info_block->set_plugin_name(plugin_name);
	}
}
const char *OpenraspAgentManager::get_plugin_name()
{
	if (plugin_rwlock != nullptr && plugin_rwlock->read_lock() && plugin_info_block)
	{
		ReadUnLocker auto_unlocker(plugin_rwlock);
		return plugin_info_block->get_plugin_name();
	}
	return "";
}

void OpenraspAgentManager::set_plugin_md5(const char *plugin_md5)
{
	if (plugin_rwlock != nullptr && plugin_rwlock->write_lock() && plugin_info_block)
	{
		WriteUnLocker auto_unlocker(plugin_rwlock);
		plugin_info_block->set_plugin_md5(plugin_md5);
	}
}
const char *OpenraspAgentManager::get_plugin_md5()
{
	if (plugin_rwlock != nullptr && plugin_rwlock->read_lock() && plugin_info_block)
	{
		ReadUnLocker auto_unlocker(plugin_rwlock);
		return plugin_info_block->get_plugin_md5();
	}
	return "";
}

long OpenraspAgentManager::get_plugin_update_timestamp()
{
	if (plugin_rwlock != nullptr && plugin_rwlock->read_lock() && plugin_info_block)
	{
		ReadUnLocker auto_unlocker(plugin_rwlock);
		return plugin_info_block->get_last_update_time();
	}
	return 0;
}

/*webdir block*/
void OpenraspAgentManager::set_webdir_scan_regex(const char *webdir_scan_regex)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		webdir_ctrl_block->set_webdir_scan_regex(webdir_scan_regex);
	}
}
const char *OpenraspAgentManager::get_webdir_scan_regex()
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return webdir_ctrl_block->get_webdir_scan_regex();
	}
	return "";
}

bool OpenraspAgentManager::path_writable()
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_try_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return strcmp(webdir_ctrl_block->get_webroot_path(), "") == 0;
	}
	return false;
}
bool OpenraspAgentManager::path_exist(ulong hash)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_try_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return webdir_ctrl_block->webroot_found(hash);
	}
	return true;
}

void OpenraspAgentManager::write_webroot_path(const char *webroot_path)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		webdir_ctrl_block->set_webroot_path(webroot_path);
	}
}

bool OpenraspAgentManager::consume_webroot_path(std::string &webroot_path)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		const char *path = webdir_ctrl_block->get_webroot_path();
		if (strlen(path) > 0)
		{
			ulong webroot_hash = zend_inline_hash_func(path, strlen(path));
			int count = webdir_ctrl_block->get_webroot_count();
			webdir_ctrl_block->set_webroot_hash(count, webroot_hash);
			webdir_ctrl_block->set_webroot_count(count + 1);
			webroot_path = std::string(path);
			webdir_ctrl_block->set_webroot_path("");
			return true;
		}
	}
	return false;
}

int OpenraspAgentManager::get_dependency_interval()
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return webdir_ctrl_block->get_dependency_interval();
	}
	return WebdirCtrlBlock::default_dependency_interval;
}

void OpenraspAgentManager::set_dependency_interval(int dependency_interval)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		webdir_ctrl_block->set_dependency_interval(dependency_interval);
	}
}

int OpenraspAgentManager::get_webdir_scan_interval()
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return webdir_ctrl_block->get_webdir_scan_interval();
	}
	return WebdirCtrlBlock::default_webdir_scan_interval;
}

void OpenraspAgentManager::set_webdir_scan_interval(int webdir_scan_interval)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		webdir_ctrl_block->set_webdir_scan_interval(webdir_scan_interval);
	}
}

long OpenraspAgentManager::get_scan_limit()
{
	if (webdir_rwlock != nullptr && webdir_rwlock->read_lock() && webdir_ctrl_block)
	{
		ReadUnLocker auto_unlocker(webdir_rwlock);
		return webdir_ctrl_block->get_scan_limit();
	}
	return WebdirCtrlBlock::default_scan_limit;
}

void OpenraspAgentManager::set_scan_limit(long scan_limit)
{
	if (webdir_rwlock != nullptr && webdir_rwlock->write_lock() && webdir_ctrl_block)
	{
		WriteUnLocker auto_unlocker(webdir_rwlock);
		webdir_ctrl_block->set_scan_limit(scan_limit);
	}
}

} // namespace openrasp
