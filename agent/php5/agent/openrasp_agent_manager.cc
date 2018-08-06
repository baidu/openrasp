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
#include <functional>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>

extern "C"
{
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
} agent_info;

static agent_info plugin_agent_info{PLUGIN_AGENT_PR_NAME, false, 0};
static agent_info log_agent_info{LOG_AGENT_PR_NAME, false, 0};
volatile static int signal_received = 0;

inline static bool need_daemon_agent()
{
	if (sapi_module.name && strcmp(sapi_module.name, "fpm-fcgi") == 0)
	{
		return true;
	}
	return false;
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

static void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins, std::function<bool(const char *filename)> file_filter)
{
	DIR *dir;
	std::string result;
	struct dirent *ent;
	if ((dir = opendir(dir_abs.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (file_filter)
			{
				if (file_filter(ent->d_name))
				{
					plugins.push_back(std::string(ent->d_name));
				}
			}
		}
		closedir(dir);
	}
}

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: _mm(mm),
	  _default_slash(1, DEFAULT_SLASH),
	  _agent_ctrl_block(NULL)
{
}

bool OpenraspAgentManager::startup()
{
	if (need_daemon_agent())
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

bool OpenraspAgentManager::create_share_memory()
{
	char *shm_block = _mm->create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock));
	if (shm_block && (_agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(shm_block)))
	{
		return true;
	}
	return false;
}

bool OpenraspAgentManager::shutdown()
{
	char buffer[MAXPATHLEN];
	pid_t ppid = getppid();
	size_t len = ::readlink(("/proc/" + std::to_string(ppid) + "/exe").c_str(), buffer, sizeof(buffer) - 1);
	if (ppid != 1 || (len != -1 && strncmp(buffer, "/sbin/init", 10)))
	{
		//fpm will shutdown twice in daemon, check symnol link for ubuntu GNOME
		//http://upstart.ubuntu.com/cookbook/#session-init
		return true;
	}
	if (initialized)
	{
		pid_t supervisor_id = static_cast<pid_t>(_agent_ctrl_block->get_supervisor_id());
		pid_t plugin_agent_id = _agent_ctrl_block->get_plugin_agent_id();
		pid_t log_agent_id = _agent_ctrl_block->get_log_agent_id();
		kill(supervisor_id, SIGKILL);
		kill(plugin_agent_id, SIGKILL);
		kill(log_agent_id, SIGKILL);
		destroy_share_memory();
	}
	return true;
}

bool OpenraspAgentManager::destroy_share_memory()
{
	this->_mm->destroy(SHMEM_SEC_CTRL_BLOCK);
	return true;
}

bool OpenraspAgentManager::process_agent_startup()
{
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
	install_signal_handler();
	TSRMLS_FETCH();
	CURL *curl = curl_easy_init();
	ResponseInfo res_info;
	while (true)
	{
		for (int i = 0; i < openrasp_ini.plugin_update_interval; ++i)
		{
			sleep(1);
			if (signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				agent_exit();
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

static inline bool is_same_day(long src, long target, long offset)
{
	long day = 24 * 60 * 60;
	return ((src + offset) / day == (target + offset) / day);
}

void OpenraspAgentManager::log_agent_run()
{
	TSRMLS_FETCH();
	install_signal_handler();

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
		bool file_rotate = !is_same_day(now, last_post_time, time_offset);
		for (LogDirInfo *ldi : log_dirs)
		{
			if (!ldi->ifs.is_open())
			{
				ldi->ifs.open(ldi->dir_abs_path + _default_slash + ldi->prefix + formatted_date_suffix);
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
				std::string tobe_deleted_date_suffix = get_formatted_date_suffix(now - 90 * 24 * 60 * 60);
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
			if (signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				agent_exit();
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
