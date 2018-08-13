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
#include "cereal/archives/binary.hpp"
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

void BaseAgent::install_signal_handler(__sighandler_t signal_handler)
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
	install_signal_handler(
			[](int signal_no) {
				PluginAgent::signal_received = signal_no;
			});
	TSRMLS_FETCH();
	CURL *curl = nullptr;
	ResponseInfo res_info;
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
		std::string url_string = std::string(openrasp_ini.backend) + "/v1/plugin?version=" + std::string(oam.agent_ctrl_block->get_plugin_version());
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
								update_local_offcial_plugin(root_dir + "/plugins/" + std::string(version) + ".js", plugin, version);
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

void PluginAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam.agent_ctrl_block->set_plugin_agent_id(agent_pid);
}

std::string PluginAgent::clear_old_offcial_plugins()
{
	TSRMLS_FETCH();
	std::string root_dir = std::string(openrasp_ini.root_dir);
	std::string plugin_dir = root_dir + "/plugins";
	std::vector<std::string> offcial_plugins;
	openrasp_scandir(plugin_dir, offcial_plugins,
									 [](const char *filename) { return !strncmp(filename, "official-", strlen("official-")) &&
																										 !strcmp(filename + strlen(filename) - 3, ".js"); });
	std::sort(offcial_plugins.rbegin(), offcial_plugins.rend());
	std::string newest_plugin;
	for (int i = 0; i < offcial_plugins.size(); ++i)
	{
		std::string plugin_abs_path = plugin_dir + default_slash + offcial_plugins[i];
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

void PluginAgent::update_local_offcial_plugin(std::string plugin_abs_path, const char *plugin, const char *version)
{
	TSRMLS_FETCH();
	std::ofstream out_file(plugin_abs_path, std::ofstream::in | std::ofstream::out | std::ofstream::trunc);
	if (out_file.is_open() && out_file.good())
	{
		out_file << plugin;
		out_file.close();
		oam.agent_ctrl_block->set_plugin_version(version);
	}
	clear_old_offcial_plugins();
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
	char *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), timestamp, 1 TSRMLS_CC);
	result = std::string(tmp_formatted_date_suffix);
	efree(tmp_formatted_date_suffix);
	return result;
}

void LogAgent::run()
{
	TSRMLS_FETCH();
	install_signal_handler(
			[](int signal_no) {
				LogAgent::signal_received = signal_no;
			});
	std::string root_dir = std::string(openrasp_ini.root_dir);
	static const std::string position_backup_file = ".LogCollectingPos";
	long last_post_time = 0;
	long time_offset = fetch_time_offset(TSRMLS_C);
	std::string formatted_date_suffix = get_formatted_date_suffix((long)time(NULL));

	std::string buffer;
	std::string line;
	ResponseInfo res_info;
	CURL *curl = nullptr;

	LogDirInfo alarm_dir_info(root_dir + default_slash + "logs" + default_slash + ALARM_LOG_DIR_NAME, "alarm.log.", "/v1/log/attack");
	LogDirInfo policy_dir_info(root_dir + default_slash + "logs" + default_slash + POLICY_LOG_DIR_NAME, "policy.log.", "/v1/log/policy");
	std::vector<LogDirInfo *> log_dirs{&alarm_dir_info, &policy_dir_info};
	try
	{
		std::ifstream is(root_dir + default_slash + "logs" + default_slash + position_backup_file);
		cereal::BinaryInputArchive archive(is);
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
		for (LogDirInfo *ldi : log_dirs)
		{
			std::string active_log_file = ldi->dir_abs_path + default_slash + ldi->prefix + formatted_date_suffix;
			if (VCWD_ACCESS(active_log_file.c_str(), F_OK) == 0)
			{
				if (!ldi->ifs.is_open())
				{
					ldi->ifs.open(active_log_file);
				}
				ldi->ifs.seekg(ldi->fpos);
				if (ldi->ifs.good())
				{
					std::string url_string = std::string(openrasp_ini.backend) + ldi->backend_url;
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
							ldi->fpos = ldi->ifs.tellg();
						}
					}
					buffer.clear();
				}
				else
				{
					ldi->ifs.seekg(0, std::ios_base::end);
					ldi->fpos = ldi->ifs.tellg();
				}
				ldi->ifs.clear();
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
			std::ofstream os(root_dir + default_slash + "logs" + default_slash + position_backup_file);
			cereal::BinaryOutputArchive archive(os);
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
} // namespace openrasp
