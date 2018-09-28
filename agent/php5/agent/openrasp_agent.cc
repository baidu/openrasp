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

#include "openrasp_agent.h"
#include "utils/digest.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include "cereal/archives/xml.hpp"
#include "cereal/types/string.hpp"
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

volatile int PluginAgent::signal_received = 0;
volatile int LogAgent::signal_received = 0;

BaseAgent::BaseAgent(std::string name)
	: default_slash(1, DEFAULT_SLASH)
{
	this->name = name;
	this->is_alive = false;
}

void BaseAgent::install_signal_handler(sighandler_t signal_handler)
{
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa_usr, NULL);
}

PluginAgent::PluginAgent()
	: BaseAgent(PLUGIN_AGENT_PR_NAME)
{
}

void PluginAgent::run()
{
	AGENT_SET_PROC_NAME(this->name.c_str());
	install_signal_handler(
		[](int signal_no) {
			PluginAgent::signal_received = signal_no;
		});
	TSRMLS_FETCH();
	CURL *curl = nullptr;
	std::string root_dir = std::string(openrasp_ini.root_dir);
	while (true)
	{
		if (nullptr == curl)
		{
			curl = curl_easy_init();
		}
		for (int i = 0; i < openrasp_ini.plugin_update_interval; ++i)
		{
			sleep(1);
			if (PluginAgent::signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				exit(0);
			}
		}
		ResponseInfo res_info;
		std::string url_string = std::string(openrasp_ini.backend_url) + "/v1/plugin";
		zval *body = nullptr;
		MAKE_STD_ZVAL(body);
		array_init(body);
		add_assoc_string(body, "version", (char *)oam.agent_ctrl_block->get_plugin_version(), 1);
		smart_str buf_json = {0};
		php_json_encode(&buf_json, body, 0 TSRMLS_CC);
		if (buf_json.a > buf_json.len)
		{
			buf_json.c[buf_json.len] = '\0';
			buf_json.len++;
		}
		perform_curl(curl, url_string, buf_json.c, res_info);
		smart_str_free(&buf_json);
		zval_ptr_dtor(&body);
		if (CURLE_OK != res_info.res)
		{
			continue;
		}
		zval *return_value = nullptr;
		MAKE_STD_ZVAL(return_value);
		php_json_decode(return_value, (char *)res_info.response_string.c_str(), res_info.response_string.size(), 1, 512 TSRMLS_CC);
		if (Z_TYPE_P(return_value) != IS_ARRAY)
		{
			zval_ptr_dtor(&return_value);
			continue;
		}
		if (res_info.response_code >= 200 && res_info.response_code < 300)
		{
			long status;
			bool has_status = fetch_outmost_long_from_ht(Z_ARRVAL_P(return_value), "status", &status);
			char *description = fetch_outmost_string_from_ht(Z_ARRVAL_P(return_value), "description");
			if (has_status && description)
			{
				if (0 < status)
				{
					openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update official plugin, status: %ld."), status);
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
								update_local_official_plugin(root_dir + "/plugins/official.js", plugin, version);
							}
						}
					}
				}
			}
		}
		else
		{
			openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to update official plugin, response code: %ld."), res_info.response_code);
		}
		zval_ptr_dtor(&return_value);
	}
}

void PluginAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam.agent_ctrl_block->set_plugin_agent_id(agent_pid);
}

void PluginAgent::update_local_official_plugin(std::string plugin_abs_path, const char *plugin, const char *version)
{
	TSRMLS_FETCH();
	std::ofstream out_file(plugin_abs_path, std::ofstream::in | std::ofstream::out | std::ofstream::trunc);
	if (out_file.is_open() && out_file.good())
	{
		out_file << plugin;
		out_file.close();
		oam.agent_ctrl_block->set_plugin_version(version);
	}
	else
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to write official plugin to %s."), plugin_abs_path.c_str());
	}
}

LogAgent::LogAgent()
	: BaseAgent(LOG_AGENT_PR_NAME)
{
}

void LogAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam.agent_ctrl_block->set_log_agent_id(agent_pid);
}

std::string LogAgent::get_formatted_date_suffix(long timestamp)
{
	TSRMLS_FETCH();
	std::string result;
	char *tmp_formatted_date_suffix = openrasp_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), timestamp);
	result = std::string(tmp_formatted_date_suffix);
	efree(tmp_formatted_date_suffix);
	return result;
}

void LogAgent::run()
{
	AGENT_SET_PROC_NAME(this->name.c_str());
	TSRMLS_FETCH();
	install_signal_handler(
		[](int signal_no) {
			LogAgent::signal_received = signal_no;
		});
	std::string root_dir = std::string(openrasp_ini.root_dir);
	static const std::string position_backup_file = ".LogCollectingPos.xml";
	long last_post_time = (long)time(NULL);
	long time_offset = fetch_time_offset();
	std::string formatted_date_suffix = get_formatted_date_suffix((long)time(NULL));

	std::string buffer;
	std::string line;
	CURL *curl = nullptr;

	LogDirInfo alarm_dir_info(root_dir + default_slash + "logs" + default_slash + ALARM_LOG_DIR_NAME, "alarm.log.", "/v1/log/attack");
	LogDirInfo policy_dir_info(root_dir + default_slash + "logs" + default_slash + POLICY_LOG_DIR_NAME, "policy.log.", "/v1/log/policy");
	LogDirInfo plugin_dir_info(root_dir + default_slash + "logs" + default_slash + PLUGIN_LOG_DIR_NAME, "plugin.log.", "/v1/log/plugin");
	LogDirInfo rasp_dir_info(root_dir + default_slash + "logs" + default_slash + RASP_LOG_DIR_NAME, "rasp.log.", "/v1/log/rasp");
	std::vector<LogDirInfo *> log_dirs{&alarm_dir_info, &policy_dir_info};
	try
	{
		std::ifstream is(root_dir + default_slash + "logs" + default_slash + position_backup_file);
		cereal::XMLInputArchive archive(is);
		archive(formatted_date_suffix, alarm_dir_info.fpos, policy_dir_info.fpos);
	}
	catch (std::exception &e)
	{
		//first startup throw excetion
	}
	while (true)
	{
		if (nullptr == curl)
		{
			curl = curl_easy_init();
		}
		long now = (long)time(NULL);
		bool file_rotate = !same_day_in_current_timezone(now, last_post_time, time_offset);
		for (int i = 0; i < log_dirs.size(); ++i)
		{
			LogDirInfo *ldi = log_dirs[i];
			std::string active_log_file = ldi->dir_abs_path + default_slash + ldi->prefix + formatted_date_suffix;
			if (VCWD_ACCESS(active_log_file.c_str(), F_OK) == 0)
			{
				if (!ldi->ifs.is_open())
				{
					ldi->ifs.open(active_log_file, std::ifstream::binary);
					ldi->st_ino = get_file_st_ino(active_log_file TSRMLS_CC);
				}
				ldi->ifs.seekg(ldi->fpos);
				if (!ldi->ifs.good())
				{
					ldi->ifs.clear();
				}
				std::string url_string = std::string(openrasp_ini.backend_url) + ldi->backend_url;
				buffer.push_back('[');
				int count = 0;
				while (std::getline(ldi->ifs, line) && count < max_post_logs_account)
				{
					buffer.append(line);
					buffer.push_back(',');
					++count;
				}
				buffer.pop_back();
				buffer.push_back(']');
				if (buffer.size() > 1)
				{
					if (post_logs_via_curl(buffer, curl, url_string))
					{
						ldi->ifs.clear();
						ldi->fpos = ldi->ifs.tellg();
					}
				}
				buffer.clear();
				long st_ino = get_file_st_ino(active_log_file TSRMLS_CC);
				if (0 != st_ino && st_ino != ldi->st_ino)
				{
					ldi->ifs.close();
					ldi->ifs.clear();
					ldi->fpos = 0;
				}
			}
			if (file_rotate)
			{
				if (ldi->ifs.is_open())
				{
					ldi->ifs.close();
					ldi->ifs.clear();
				}
				ldi->fpos = 0;
			}
		}
		if (file_rotate)
		{
			std::vector<LogDirInfo *> tobe_cleaned_logdirs{&alarm_dir_info, &policy_dir_info, &plugin_dir_info, &rasp_dir_info};
			cleanup_expired_logs(tobe_cleaned_logdirs);
			formatted_date_suffix = get_formatted_date_suffix((long)time(NULL));
		}
		last_post_time = now;
		try
		{
			std::ofstream os(root_dir + default_slash + "logs" + default_slash + position_backup_file);
			cereal::XMLOutputArchive archive(os);
			archive(formatted_date_suffix, alarm_dir_info.fpos, policy_dir_info.fpos);
		}
		catch (std::exception &e)
		{
		}
		for (int i = 0; i < openrasp_ini.log_push_interval; ++i)
		{
			sleep(1);
			if (LogAgent::signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				exit(0);
			}
		}
	}
}

bool LogAgent::post_logs_via_curl(std::string log_arr, CURL *curl, std::string url_string)
{
	ResponseInfo res_info;
	perform_curl(curl, url_string, log_arr.c_str(), res_info);
	if (CURLE_OK != res_info.res ||
		res_info.response_code < 200 && res_info.response_code >= 300)
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to post logs to %s."), url_string.c_str());
		return false;
	}
	return true;
}

void LogAgent::cleanup_expired_logs(std::vector<LogDirInfo *> &tobe_cleaned_logdirs)
{
	TSRMLS_FETCH();
	long now = (long)time(NULL);
	for (auto item : tobe_cleaned_logdirs)
	{
		std::vector<std::string> files_tobe_deleted;
		std::string tobe_deleted_date_suffix = get_formatted_date_suffix(now - openrasp_ini.log_max_backup * 24 * 60 * 60);
		openrasp_scandir(item->dir_abs_path, files_tobe_deleted,
						 [&item, &tobe_deleted_date_suffix](const char *filename) {
							 return !strncmp(filename, item->prefix.c_str(), item->prefix.size()) &&
									std::string(filename) < (item->prefix + tobe_deleted_date_suffix);
						 },
						 true);
		for (std::string delete_file : files_tobe_deleted)
		{
			VCWD_UNLINK(delete_file.c_str());
		}
	}
}
} // namespace openrasp
