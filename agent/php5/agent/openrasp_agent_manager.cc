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
#include "openrasp_agent_manager.h"
#include "openrasp_log_collector.h"
#include <string>
#include <vector>
#include <iostream>
#include "openrasp_ini.h"
#include "utils/digest.h"
#include <thread>
#include <dirent.h>
#include <algorithm>
#include <functional>

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

typedef bool (*file_filter_t)(const char *filename);

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

static void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins, file_filter_t file_filter)
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

static inline char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey)
{
	zval **origin_zv;
	if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS &&
		Z_TYPE_PP(origin_zv) == IS_STRING)
	{
		return Z_STRVAL_PP(origin_zv);
	}
	return nullptr;
}

OpenraspAgentManager::OpenraspAgentManager(ShmManager *mm)
	: _mm(mm),
	  _default_slash(1, DEFAULT_SLASH),
	  _agent_ctrl_block(NULL)
{
}

int OpenraspAgentManager::startup()
{
	if (need_daemon_agent())
	{
		_root_dir = std::string(openrasp_ini.root_dir);
		_backend = std::string(openrasp_ini.backend);
		create_share_memory();
		offcial_plugin_version_shm();
		agent_startup();
		initialized = true;
	}
	return SUCCESS;
}

int OpenraspAgentManager::create_share_memory()
{
	_agent_ctrl_block = reinterpret_cast<OpenraspCtrlBlock *>(
		_mm->create(SHMEM_SEC_CTRL_BLOCK, sizeof(OpenraspCtrlBlock)));
	return SUCCESS;
}

int OpenraspAgentManager::shutdown()
{
	if (initialized)
	{
		pid_t supervisor_id = static_cast<pid_t>(_agent_ctrl_block->get_supervisor_id());
		pid_t plugin_agent_id = _agent_ctrl_block->get_plugin_agent_id();
		pid_t log_agent_id = _agent_ctrl_block->get_log_agent_id();
		kill(supervisor_id, SIGTERM);
		kill(plugin_agent_id, SIGTERM);
		kill(log_agent_id, SIGTERM);
		destroy_share_memory();
	}
	return SUCCESS;
}

int OpenraspAgentManager::destroy_share_memory()
{
	this->_mm->destroy(SHMEM_SEC_CTRL_BLOCK);
	return SUCCESS;
}

int OpenraspAgentManager::agent_startup()
{
	process_agent_startup();
	return SUCCESS;
}

int OpenraspAgentManager::process_agent_startup()
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

std::string OpenraspAgentManager::clear_offcial_plugins()
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

int OpenraspAgentManager::offcial_plugin_version_shm()
{
	TSRMLS_FETCH();
	std::string newest_plugin = clear_offcial_plugins();
	if (!newest_plugin.empty())
	{
		size_t pos = newest_plugin.find(".js");
		if (pos != std::string::npos)
		{
			_agent_ctrl_block->set_plugin_version(std::string(newest_plugin, 0, pos).c_str());
		}
	}
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
		sleep(1);
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
	clear_offcial_plugins();
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
			char *version = nullptr;
			char *plugin = nullptr;
			char *md5 = nullptr;
			if ((version = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "version")) &&
				(plugin = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "plugin")) &&
				(md5 = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "md5")))
			{
				std::string cal_md5 = md5sum(static_cast<const void *>(plugin), strlen(plugin));
				if (!strcmp(cal_md5.c_str(), md5))
				{
					update_local_offcial_plugin(_root_dir + "/plugins/" + std::string(version) + ".js", plugin, version);
				}
			}
		}
		else
		{
			char *error = nullptr;
			if ((error = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "error")))
			{
				openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update offcial plugin, error: %s."), error);
			}
		}
		zval_ptr_dtor(&return_value);
	}
}

std::string OpenraspAgentManager::update_formatted_date_suffix()
{
	TSRMLS_FETCH();
	std::string result;
	char *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), (long)time(NULL), 1 TSRMLS_CC);
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

	long last_post_time = 0;
	long time_offset = fetch_time_offset();
	std::string formatted_date_suffix =	update_formatted_date_suffix();

	zval *log_arr = nullptr;
	MAKE_STD_ZVAL(log_arr);
	array_init(log_arr);

	std::string line;
	ResponseInfo res_info;
	CURL *curl = curl_easy_init();

	std::vector<LogDirInfo> log_dirs;
	log_dirs.push_back(std::move(LogDirInfo(_root_dir + _default_slash + "logs" + _default_slash + ALARM_LOG_DIR_NAME, "alarm.log.", "/v1/log/attack")));
	log_dirs.push_back(std::move(LogDirInfo(_root_dir + _default_slash + "logs" + _default_slash + POLICY_LOG_DIR_NAME, "policy.log.", "/v1/log/policy")));
	while (true)
	{
		long now = (long)time(NULL);
		bool file_rotate = !is_same_day(now, last_post_time, time_offset);
		if (file_rotate)
		{
			formatted_date_suffix =	update_formatted_date_suffix();
		}
		for (int i = 0; i < log_dirs.size(); ++i)
		{
			if (log_dirs[i].ifs.is_open() && log_dirs[i].ifs.good())
			{
				std::string url_string = _backend + log_dirs[i].backend_url;
				while (std::getline(log_dirs[i].ifs, line))
				{
					add_next_index_stringl(log_arr, line.c_str(), line.size(), 1);
				}
				log_dirs[i].ifs.clear();
				if (zend_hash_num_elements(Z_ARRVAL_P(log_arr)) > 0)
				{
					post_logs_via_curl(log_arr, curl, url_string);
					zend_hash_clean(Z_ARRVAL_P(log_arr));
				}
			}
			if (file_rotate)
			{
				if (log_dirs[i].ifs.is_open())
				{
					log_dirs[i].ifs.close();
					log_dirs[i].ifs.clear();
				}
			}
			if (!log_dirs[i].ifs.is_open())
			{
				log_dirs[i].ifs.open(log_dirs[i].dir_abs_path + _default_slash + log_dirs[i].prefix + formatted_date_suffix);
				log_dirs[i].ifs.seekg(0, 0 == last_post_time ? std::ios_base::end : std::ios_base::beg);
			}
		}
		last_post_time = now;
		for (int i = 0; i < openrasp_ini.log_push_interval; ++i)
		{
			sleep(1);
			if (signal_received == SIGTERM)
			{
				zval_ptr_dtor(&log_arr);
				curl_easy_cleanup(curl);
				curl = nullptr;
				agent_exit();
			}
		}
	}
}

void OpenraspAgentManager::post_logs_via_curl(zval *log_arr, CURL *curl, std::string url_string)
{
	TSRMLS_FETCH();
	smart_str buf_json = {0};
	php_json_encode(&buf_json, log_arr, 0 TSRMLS_CC);
	ResponseInfo res_info;
	if (buf_json.a > buf_json.len)
	{
		buf_json.c[buf_json.len] = '\0';
		buf_json.len++;
	}
	perform_curl(curl, url_string, buf_json.c, res_info);
	if (CURLE_OK != res_info.res)
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to post logs to %s."), url_string.c_str());
	}
	smart_str_free(&buf_json);
}

} // namespace openrasp
