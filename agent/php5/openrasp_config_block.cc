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

#include "openrasp_config_block.h"

namespace openrasp
{
void PluginBlock::update(OpenraspConfig *openrasp_config)
{
  timeout.millis = openrasp_config->Get("plugin.timeout.millis", (int64_t)100);
  maxstack = openrasp_config->Get("plugin.maxstack", (int64_t)100);
  filter = openrasp_config->Get("plugin.filter", true);
};

void LogBlock::update(OpenraspConfig *openrasp_config)
{
  maxburst = openrasp_config->Get("log.maxburst", (int64_t)100);
  maxstack = openrasp_config->Get("log.maxstack", (int64_t)10);
};

void SyslogBlock::update(OpenraspConfig *openrasp_config)
{
  tag = openrasp_config->Get("syslog.tag", std::string("OpenRASP"));
  url = openrasp_config->Get("syslog.url", std::string(""));
  facility = openrasp_config->Get("syslog.facility", (int64_t)1);
  enable = openrasp_config->Get("syslog.enable", false);
  connection_timeout = openrasp_config->Get("syslog.connection_timeout", (int64_t)50);
  read_timeout = openrasp_config->Get("syslog.read_timeout", (int64_t)10);
  reconnect_interval = openrasp_config->Get("syslog.reconnect_interval", (int64_t)300);
};

void BlockBlock::update(OpenraspConfig *openrasp_config)
{
  status_code = openrasp_config->Get("block.status_code", (int64_t)302);
  redirect_url = openrasp_config->Get("block.redirect_url", std::string(R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)"));
  content_json = openrasp_config->Get("block.content_json", std::string(R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})"));
  content_xml = openrasp_config->Get("block.content_xml", std::string(R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)"));
  content_html = openrasp_config->Get("block.content_html", std::string(R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)"));
};

void InjectBlock::update(OpenraspConfig *openrasp_config)
{
  urlprefix = openrasp_config->Get("inject.urlprefix", std::string(""));
};

void BodyBlock::update(OpenraspConfig *openrasp_config)
{
  maxbytes = openrasp_config->Get("body.maxbytes", (int64_t)4 * 1024);
};

void ClientipBlock::update(OpenraspConfig *openrasp_config)
{
  header = openrasp_config->Get("clientip.header", std::string(""));
};

void SecurityBlock::update(OpenraspConfig *openrasp_config)
{
  enforce_policy = openrasp_config->Get("security.enforce_policy", false);
};

void SqlBlock::update(OpenraspConfig *openrasp_config)
{
  slowquery.min_rows = openrasp_config->Get("sql.slowquery.min_rows", (int64_t)500);
};

void LruBlock::update(OpenraspConfig *openrasp_config)
{
  max_size = openrasp_config->Get("lru.max_size", (int64_t)1024);
};

const vector<string> CallableBlock::default_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};

void CallableBlock::update(OpenraspConfig *openrasp_config)
{
  blacklist = openrasp_config->GetArray("callable.blacklist", CallableBlock::default_blacklist);
}

} // namespace openrasp