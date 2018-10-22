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

#include "openrasp_config.h"
#include "openrasp_agent.h"
#include "openrasp_hook.h"
#include "utils/digest.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include "shared_config_manager.h"
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
	TSRMLS_FETCH();
	CURL *curl = curl_easy_init();
	while (true)
	{
		do_heartbeat(curl TSRMLS_CC);
		for (int i = 0; i < HeartBeatAgent::plugin_update_interval; ++i)
		{
			sleep(1);
			if (HeartBeatAgent::signal_received == SIGTERM)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
				exit(0);
			}
		}
	}
}

void HeartBeatAgent::do_heartbeat(CURL *curl TSRMLS_DC)
{
	if (nullptr == curl)
	{
		curl = curl_easy_init();
		if (nullptr == curl)
		{
			return;
		}
	} //make sure curl is not nullptr
	ResponseInfo res_info;
	std::string url_string = std::string(openrasp_ini.backend_url) + "/v1/agent/heartbeat";
	zval *body = nullptr;
	MAKE_STD_ZVAL(body);
	array_init(body);
	add_assoc_string(body, "rasp_id", (char *)oam->get_rasp_id().c_str(), 1);
	add_assoc_string(body, "plugin_version", (char *)oam->agent_ctrl_block->get_plugin_version(), 1);
	add_assoc_long(body, "config_time", (scm ? scm->get_config_last_update() : 0));
	std::string request_body = json_encode_from_zval(body TSRMLS_CC);
	perform_curl(curl, url_string, request_body.c_str(), res_info);
	zval_ptr_dtor(&body);
	if (CURLE_OK != res_info.res)
	{
		return;
	}
	zval *return_value = nullptr;
	MAKE_STD_ZVAL(return_value);
	php_json_decode(return_value, (char *)res_info.response_string.c_str(), res_info.response_string.size(), 1, 512 TSRMLS_CC);
	if (Z_TYPE_P(return_value) != IS_ARRAY)
	{
		zval_ptr_dtor(&return_value);
		return;
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
				openrasp_error(E_WARNING, AGENT_ERROR, _("Heartbeat error, status: %ld, description :%s."), status, description);
			}
			else if (0 == status)
			{
				HashTable *data = fetch_outmost_hashtable_from_ht(Z_ARRVAL_P(return_value), "data");
				if (data)
				{
					bool has_new_plugin = false;
					bool has_new_algorithm_config = false;
					HashTable *plugin_ht = nullptr;
					if (plugin_ht = fetch_outmost_hashtable_from_ht(data, "plugin"))
					{
						has_new_plugin = update_official_plugin(plugin_ht);
					}
					long config_time;
					bool has_config_time = fetch_outmost_long_from_ht(data, "config_time", &config_time);
					zval *config_zv = fetch_outmost_zval_from_ht(data, "config");
					if (has_config_time && config_zv)
					{
						update_config(config_zv, config_time, &has_new_algorithm_config TSRMLS_CC);
					}
					if (has_new_plugin || has_new_algorithm_config)
					{
						if (build_plugin_snapshot(TSRMLS_C))
						{
							oam->agent_ctrl_block->set_plugin_version(fetch_outmost_string_from_ht(plugin_ht, "version"));
						}
					}
				}
			}
		}
	}
	else
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Heartbeat error, response code: %ld."), res_info.response_code);
	}
	zval_ptr_dtor(&return_value);
}

bool HeartBeatAgent::update_official_plugin(HashTable *plugin_ht)
{
	assert(plugin_ht != nullptr);
	char *version = nullptr;
	char *plugin = nullptr;
	char *md5 = nullptr;
	if ((version = fetch_outmost_string_from_ht(plugin_ht, "version")) &&
		(plugin = fetch_outmost_string_from_ht(plugin_ht, "plugin")) &&
		(md5 = fetch_outmost_string_from_ht(plugin_ht, "md5")))
	{
		std::string cal_md5 = md5sum(static_cast<const void *>(plugin), strlen(plugin));
		if (!strcmp(cal_md5.c_str(), md5))
		{
			active_plugins.clear();
			active_plugins.push_back({"official.js", std::string(plugin)});
			return true;
		}
	}
	return false;
}

bool HeartBeatAgent::update_config(zval *config_zv, long config_time, bool *has_new_algorithm_config TSRMLS_DC)
{
	if (Z_TYPE_P(config_zv) != IS_ARRAY)
	{
		return false;
	}
	zval *algorithm_config_zv = fetch_outmost_zval_from_ht(Z_ARRVAL_P(config_zv), "algorithm.config");
	if (algorithm_config_zv)
	{
		algorithm_config = "RASP.algorithmConfig=" + json_encode_from_zval(algorithm_config_zv TSRMLS_CC);
		*has_new_algorithm_config = true;
	}
	zend_hash_del(Z_ARRVAL_P(config_zv), "algorithm.config", sizeof("algorithm.config"));
	if (zend_hash_num_elements(Z_ARRVAL_P(config_zv)) <= 0)
	{
		return true;
	}
	std::string config_string = json_encode_from_zval(config_zv TSRMLS_CC);
	OpenraspConfig openrasp_config(config_string, OpenraspConfig::FromType::kJson);
	if (scm != nullptr)
	{
		scm->build_check_type_white_array(openrasp_config);
		//update log_max_backup only its value greater than zero
		long log_max_backup = openrasp_config.Get("log.maxbackup", openrasp_config.log.maxbackup);
		if (log_max_backup)
		{
			scm->set_log_max_backup(log_max_backup);
		}
	}
	std::string cloud_config_file_path = std::string(openrasp_ini.root_dir) + "/conf/cloud-config.json";
#ifndef _WIN32
	mode_t oldmask = umask(0);
#endif
	bool write_ok = write_str_to_file(cloud_config_file_path.c_str(),
									  std::ofstream::in | std::ofstream::out | std::ofstream::trunc,
									  config_string.c_str(),
									  config_string.length());
#ifndef _WIN32
	umask(oldmask);
#endif
	if (write_ok)
	{
		scm->set_config_last_update(config_time);
	}
	else
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to write cloud config to %s."), cloud_config_file_path.c_str());
		return false;
	}
	return true;
}

bool HeartBeatAgent::build_plugin_snapshot(TSRMLS_D)
{
	Platform::Initialize();
	Snapshot snapshot(algorithm_config, active_plugins);
	Platform::Shutdown();
	if (!snapshot.IsOk())
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to generate snapshot."));
		return false;
	}
	std::string snapshot_abs_path = std::string(openrasp_ini.root_dir) + "/snapshot.dat";
#ifndef _WIN32
	mode_t oldmask = umask(0);
#endif
	bool write_successful = snapshot.Save(snapshot_abs_path);
#ifndef _WIN32
	umask(oldmask);
#endif
	if (!write_successful)
	{
		openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to write snapshot to %s."), snapshot_abs_path.c_str());
	}
	return write_successful;
}

void HeartBeatAgent::write_pid_to_shm(pid_t agent_pid)
{
	oam->agent_ctrl_block->set_plugin_agent_id(agent_pid);
}

} // namespace openrasp
