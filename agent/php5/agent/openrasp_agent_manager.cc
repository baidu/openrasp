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

#include "openrasp.h"
#include "openrasp_log.h"
#include "openrasp_agent_manager.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/digest.h"
#include <dirent.h>
#include <algorithm>
#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"

extern "C"
{
#include "openrasp_shared_alloc.h"
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
#include "ext/date/php_date.h"
#include "php_streams.h"
#include "php_main.h"
}

namespace openrasp
{

ShmManager sm;
OpenraspAgentManager oam(&sm);

typedef struct agent_info_t
{
	const std::string name;
	bool is_alive;
	unsigned long agent_pid;
	volatile int signal_received;
} agent_info;

static agent_info plugin_agent_info{PLUGIN_AGENT_PR_NAME, false, 0, 0};
static agent_info log_agent_info{LOG_AGENT_PR_NAME, false, 0, 0};

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
		if (p == plugin_agent_info.agent_pid)
		{
			plugin_agent_info.is_alive = false;
		}
		else if (p == log_agent_info.agent_pid)
		{
			log_agent_info.is_alive = false;
		}
	}
}

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: _mm(mm),
	  _default_slash(1, DEFAULT_SLASH),
	  _agent_ctrl_block(nullptr)
{
}

bool OpenraspAgentManager::startup()
{
	first_process_pid = getpid();
	if (check_sapi_need_alloc_shm())
	{
		_root_dir = std::string(openrasp_ini.root_dir);
		_backend = std::string(openrasp_ini.backend);
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
			_agent_ctrl_block->set_master_pid(master_pid);
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
	if (shm_block && (_agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_block)))
	{
		return true;
	}
	return false;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	_agent_ctrl_block = nullptr;
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

void OpenraspAgentManager::install_signal_handler(__sighandler_t signal_handler)
{
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa_usr, NULL);
}

bool OpenraspAgentManager::process_agent_startup()
{
	_agent_ctrl_block->set_master_pid(first_process_pid);
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
		_agent_ctrl_block->set_supervisor_id(pid);
	}
	return true;
}

void OpenraspAgentManager::process_agent_shutdown()
{
	pid_t supervisor_id = _agent_ctrl_block->get_supervisor_id();
	pid_t plugin_agent_id = _agent_ctrl_block->get_plugin_agent_id();
	pid_t log_agent_id = _agent_ctrl_block->get_log_agent_id();
	kill(plugin_agent_id, SIGKILL);
	kill(log_agent_id, SIGKILL);
	kill(supervisor_id, SIGKILL);
}

std::string OpenraspAgentManager::clear_old_offcial_plugins()
{
	TSRMLS_FETCH();
	std::string plugin_dir = _root_dir + "/plugins";
	std::vector<std::string> offcial_plugins;
	openrasp_scandir(plugin_dir, offcial_plugins,
					 [](const char *filename) { return !strncmp(filename, "official-", strlen("official-")) &&
													   !strcmp(filename + strlen(filename) - 3, ".js"); });
	std::sort(offcial_plugins.rbegin(), offcial_plugins.rend());
	std::string newest_plugin;
	for (int i = 0; i < offcial_plugins.size(); ++i)
	{
		std::string plugin_abs_path = plugin_dir + _default_slash + offcial_plugins[i];
		if (0 == i)
		{
			newest_plugin = offcial_plugins[i];
		}
		else
		{
			VCWD_UNLINK(plugin_abs_path.c_str());
		}
	}
	return newest_plugin;
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
				if (!plugin_agent_info.is_alive)
				{
					pid_t pid = fork();
					if (pid == 0)
					{
						plugin_agent_run();
					}
					else if (pid > 0)
					{
						plugin_agent_info.is_alive = true;
						plugin_agent_info.agent_pid = pid;
						_agent_ctrl_block->set_plugin_agent_id(pid);
					}
				}
				if (!log_agent_info.is_alive)
				{
					pid_t pid = fork();
					if (pid == 0)
					{
						log_agent_run();
					}
					else if (pid > 0)
					{
						log_agent_info.is_alive = true;
						log_agent_info.agent_pid = pid;
						_agent_ctrl_block->set_log_agent_id(pid);
					}
				}
			}
			sleep(1);
			struct stat sb;
			if (VCWD_STAT(("/proc/" + std::to_string(_agent_ctrl_block->get_master_pid())).c_str(), &sb) == -1 &&
				errno == ENOENT)
			{
				process_agent_shutdown();
			}
		}
	}
}

void OpenraspAgentManager::update_local_offcial_plugin(std::string plugin_abs_path, const char *plugin, const char *version)
{
	TSRMLS_FETCH();
	std::ofstream out_file(plugin_abs_path, std::ofstream::in | std::ofstream::out | std::ofstream::trunc);
	if (out_file.is_open() && out_file.good())
	{
		out_file << plugin;
		out_file.close();
		_agent_ctrl_block->set_plugin_version(version);
	}
	clear_old_offcial_plugins();
}

void OpenraspAgentManager::plugin_agent_run()
{
	install_signal_handler(
		[](int signal_no) {
			plugin_agent_info.signal_received = signal_no;
		});
	TSRMLS_FETCH();
	CURL *curl = curl_easy_init();
	ResponseInfo res_info;
	while (true)
	{
		for (int i = 0; i < openrasp_ini.plugin_update_interval; ++i)
		{
			sleep(1);
			if (plugin_agent_info.signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				exit(0);
			}
		}
		std::string url_string = _backend + "/v1/plugin?version=" + std::string(_agent_ctrl_block->get_plugin_version());
		perform_curl(curl, url_string, nullptr, res_info);
		if (CURLE_OK != res_info.res)
		{
			continue;
		}
		zval *return_value = nullptr;
		MAKE_STD_ZVAL(return_value);
		php_json_decode(return_value, (char *)res_info.response_string.c_str(), res_info.response_string.size(), 1, 512 TSRMLS_CC);
		zval **origin_zv;
		if (Z_TYPE_P(return_value) != IS_ARRAY)
		{
			zval_ptr_dtor(&return_value);
			continue;
		}
		if (res_info.response_code >= 200 && res_info.response_code < 300)
		{
			long status;
			char *description = nullptr;
			if ((status = fetch_outmost_long_from_ht(Z_ARRVAL_P(return_value), "status")) &&
				(description = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "description")))
			{
				if (0 < status)
				{
					openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update offcial plugin, status: %ld."), status);
				}
				else if (0 == status)
				{
					if (HashTable *data = fetch_outmost_hashtable_from_ht(Z_ARRVAL_P(return_value), "data"))
					{
						char *version = nullptr;
						char *plugin = nullptr;
						char *md5 = nullptr;
						if ((version = fetch_outmost_string_from_ht(data, "version")) &&
							(plugin = fetch_outmost_string_from_ht(data, "plugin")) &&
							(md5 = fetch_outmost_string_from_ht(data, "md5")))
						{
							std::string cal_md5 = md5sum(static_cast<const void *>(plugin), strlen(plugin));
							if (!strcmp(cal_md5.c_str(), md5))
							{
								update_local_offcial_plugin(_root_dir + "/plugins/" + std::string(version) + ".js", plugin, version);
							}
						}
					}
				}
			}
		}
		else
		{
			openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update offcial plugin, response code: %ld."), res_info.response_code);
		}
		zval_ptr_dtor(&return_value);
	}
}

std::string OpenraspAgentManager::get_formatted_date_suffix(long timestamp)
{
	TSRMLS_FETCH();
	std::string result;
	char *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), timestamp, 1 TSRMLS_CC);
	result = std::string(tmp_formatted_date_suffix);
	efree(tmp_formatted_date_suffix);
	return result;
}

void OpenraspAgentManager::log_agent_run()
{
	TSRMLS_FETCH();
	install_signal_handler(
		[](int signal_no) {
			log_agent_info.signal_received = signal_no;
		});

	static const std::string position_backup_file = ".LogCollectingPos";
	long last_post_time = 0;
	long time_offset = fetch_time_offset(TSRMLS_C);
	std::string formatted_date_suffix = get_formatted_date_suffix((long)time(NULL));

	std::string buffer;
	std::string line;
	ResponseInfo res_info;
	CURL *curl = curl_easy_init();

	LogDirInfo alarm_dir_info(_root_dir + _default_slash + "logs" + _default_slash + ALARM_LOG_DIR_NAME, "alarm.log.", "/v1/log/attack");
	LogDirInfo policy_dir_info(_root_dir + _default_slash + "logs" + _default_slash + POLICY_LOG_DIR_NAME, "policy.log.", "/v1/log/policy");
	std::vector<LogDirInfo *> log_dirs{&alarm_dir_info, &policy_dir_info};
	try
	{
		std::ifstream is(_root_dir + _default_slash + "conf" + _default_slash + position_backup_file);
		cereal::BinaryInputArchive archive(is);
		archive(formatted_date_suffix, alarm_dir_info.fpos, policy_dir_info.fpos);
	}
	catch (std::exception &e)
	{
		//first startup throw excetion
	}
	while (true)
	{
		long now = (long)time(NULL);
		bool file_rotate = !same_day_in_current_timezone(now, last_post_time, time_offset);
		for (LogDirInfo *ldi : log_dirs)
		{
			std::string active_log_file = ldi->dir_abs_path + _default_slash + ldi->prefix + formatted_date_suffix;
			if (!ldi->ifs.is_open() && VCWD_ACCESS(active_log_file.c_str(), F_OK) == 0)
			{
				ldi->ifs.open(active_log_file);
				ldi->ifs.seekg(ldi->fpos);
			}
			if (ldi->ifs.is_open() && ldi->ifs.good())
			{
				std::string url_string = _backend + ldi->backend_url;
				buffer.push_back('[');
				while (std::getline(ldi->ifs, line))
				{
					buffer.append(line);
					buffer.push_back(',');
				}
				buffer.pop_back();
				buffer.push_back(']');
				ldi->ifs.clear();
				if (buffer.size() > 1)
				{
					post_logs_via_curl(buffer, curl, url_string);
				}
				buffer.clear();
				ldi->fpos = ldi->ifs.tellg();
			}
			if (file_rotate)
			{
				if (ldi->ifs.is_open())
				{
					ldi->ifs.close();
					ldi->ifs.clear();
				}
				std::vector<std::string> files_tobe_deleted;
				std::string tobe_deleted_date_suffix = get_formatted_date_suffix(now - openrasp_ini.log_max_backup * 24 * 60 * 60);
				openrasp_scandir(ldi->dir_abs_path, files_tobe_deleted,
								 [&ldi, &tobe_deleted_date_suffix](const char *filename) {
									 return !strncmp(filename, ldi->prefix.c_str(), ldi->prefix.size()) &&
											std::string(filename) < (ldi->prefix + tobe_deleted_date_suffix);
								 });
				for (std::string delete_file : files_tobe_deleted)
				{
					VCWD_UNLINK(delete_file.c_str());
				}
			}
		}
		if (file_rotate)
		{
			formatted_date_suffix = get_formatted_date_suffix((long)time(NULL));
		}
		last_post_time = now;
		try
		{
			std::ofstream os(_root_dir + _default_slash + "conf" + _default_slash + position_backup_file);
			cereal::BinaryOutputArchive archive(os);
			archive(formatted_date_suffix, alarm_dir_info.fpos, policy_dir_info.fpos);
		}
		catch (std::exception &e)
		{
		}
		for (int i = 0; i < openrasp_ini.log_push_interval; ++i)
		{
			sleep(1);
			if (log_agent_info.signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				exit(0);
			}
		}
	}
}

void OpenraspAgentManager::post_logs_via_curl(std::string log_arr, CURL *curl, std::string url_string)
{
	ResponseInfo res_info;
	perform_curl(curl, url_string, log_arr.c_str(), res_info);
	if (CURLE_OK != res_info.res)
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to post logs to %s."), url_string.c_str());
	}
}

} // namespace openrasp
