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
#include "openrasp_hook.h"
#include "utils/digest.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include "shared_config_manager.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace openrasp
{

volatile int HeartBeatAgent::signal_received = 0;

HeartBeatAgent::HeartBeatAgent()
	: BaseAgent(HEARTBEAT_AGENT_PR_NAME)
{
}

void HeartBeatAgent::run()
{
	AGENT_SET_PROC_NAME(this->name.c_str());

	install_signal_handler(
		[](int signal_no) {
			HeartBeatAgent::signal_received = signal_no;
		});

	TS_FETCH_WRAPPER();
	while (true)
	{
		LOG_G(rasp_logger).set_level(scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
		do_heartbeat();

		for (long i = 0; i < openrasp_ini.heartbeat_interval; ++i)
		{
			sleep(1);
			if (HeartBeatAgent::signal_received == SIGTERM)
			{
				exit(0);
			}
		}
	}
}

void HeartBeatAgent::do_heartbeat()
{
	std::string url_string = std::string(openrasp_ini.backend_url) + "/v1/agent/heartbeat";

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("rasp_id");
	writer.String(scm->get_rasp_id().c_str());
	writer.Key("plugin_md5");
	writer.String(oam->agent_ctrl_block->get_plugin_md5());
	writer.Key("plugin_version");
	writer.String(oam->agent_ctrl_block->get_plugin_version());
	writer.Key("config_time");
	writer.Int64((scm->get_config_last_update()));
	writer.EndObject();

	BackendRequest backend_request(url_string, s.GetString());
	std::shared_ptr<BackendResponse> res_info = backend_request.curl_perform();
	if (!res_info)
	{
		openrasp_error(E_WARNING, HEARTBEAT_ERROR, _("CURL error code: %d, url: %s"),
					   backend_request.get_curl_code(), url_string.c_str());
		return;
	}

	if (res_info->verify(HEARTBEAT_ERROR))
	{
		/************************************plugin update************************************/
		std::shared_ptr<PluginUpdatePackage> plugin_update_pkg = res_info->build_plugin_update_package();
		if (plugin_update_pkg)
		{
			if (plugin_update_pkg->build_snapshot())
			{
				oam->agent_ctrl_block->set_plugin_md5(plugin_update_pkg->get_md5().c_str());
				oam->agent_ctrl_block->set_plugin_version(plugin_update_pkg->get_version().c_str());
			}
		}
		/************************************buildin action************************************/
		std::map<OpenRASPCheckType, OpenRASPActionType> buildin_action_map;
		const std::vector<OpenRASPCheckType> buildin_types = check_type_transfer->get_buildin_types();
		for (auto type : buildin_types)
		{
			std::string action;
			res_info->fetch_string(("/data/config/algorithm.config/" + check_type_transfer->type_to_name(type) + "/action").c_str(), action);
			buildin_action_map.insert({type, string_to_action(action)});
		}
		scm->set_buildin_check_action(buildin_action_map);
		res_info->erase_value("/data/config/algorithm.config");
		/************************************config update************************************/
		int64_t config_time;
		if (!res_info->fetch_int64("/data/config_time", config_time))
		{
			return;
		}
		std::string complete_config;
		if (res_info->stringify_object("/data/config", complete_config))
		{
			/************************************shm config************************************/
			if (scm != nullptr)
			{
				//update log_max_backup only its value greater than zero
				int64_t log_max_backup = 30;
				res_info->fetch_int64("/data/config/log.maxbackup", log_max_backup);
				scm->set_log_max_backup(log_max_backup);
				int64_t debug_level = 0;
				res_info->fetch_int64("/data/config/debug.level", debug_level);
				scm->set_debug_level(debug_level);
				std::map<std::string, std::vector<std::string>> white_map = res_info->build_hook_white_map("/data/config/hook.white");
				std::map<std::string, int> white_mask_map;
				for (auto &white_item : white_map)
				{
					int bit_mask = 0;
					if (std::find(white_item.second.begin(), white_item.second.end(), "all") != white_item.second.end())
					{
						bit_mask = (1 << ALL_TYPE - 1);
					}
					else
					{
						for (auto &type_name : white_item.second)
						{
							bit_mask |= (1 << check_type_transfer->name_to_type(type_name));
						}
					}
					white_mask_map.insert({(white_item.first == "*") ? "" : white_item.first, bit_mask});
				}
				scm->build_check_type_white_array(white_mask_map);
			}
			res_info->erase_value("/data/config/hook.white");
			res_info->erase_value("/data/config/log.maxbackup");

			/************************************OPENRASP_G(config)************************************/
			std::string exculde_hook_white_config;
			if (res_info->stringify_object("/data/config", exculde_hook_white_config))
			{
				std::string cloud_config_file_path = std::string(openrasp_ini.root_dir) + "/conf/cloud-config.json";
#ifndef _WIN32
				mode_t oldmask = umask(0);
#endif
				bool write_ok = write_str_to_file(cloud_config_file_path.c_str(),
												  std::ofstream::in | std::ofstream::out | std::ofstream::trunc,
												  exculde_hook_white_config.c_str(),
												  exculde_hook_white_config.length());
#ifndef _WIN32
				umask(oldmask);
#endif
				if (write_ok)
				{
					scm->set_config_last_update(config_time);
				}
				else
				{
					openrasp_error(E_WARNING, HEARTBEAT_ERROR, _("Fail to write cloud config to %s, error %s."),
								   cloud_config_file_path.c_str(), strerror(errno));
				}
				return;
			}
		}
		return;
	}
}

void HeartBeatAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->agent_ctrl_block->set_plugin_agent_id(agent_pid);
}

} // namespace openrasp
