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
#include "utils/regex.h"

namespace openrasp
{

void g_zero_filter(int64_t &value, const int64_t &dafault)
{
  if (value <= 0)
  {
    value = dafault;
  }
}

void ge_zero_filter(int64_t &value, const int64_t &dafault)
{
  if (value < 0)
  {
    value = dafault;
  }
}

void regex_filter(string &value, const string &regex, const string &dafault)
{
  if (!regex_match(value.c_str(), regex.c_str()))
  {
    value = dafault;
  }
}

const int64_t PluginBlock::default_timeout_millis = 100;
const int64_t PluginBlock::default_maxstack = 100;

void PluginBlock::update(OpenraspConfig *openrasp_config)
{
  timeout.millis = openrasp_config->Get("plugin.timeout.millis", PluginBlock::default_timeout_millis);
  g_zero_filter(timeout.millis, PluginBlock::default_timeout_millis);

  maxstack = openrasp_config->Get("plugin.maxstack", PluginBlock::default_maxstack);
  g_zero_filter(maxstack, PluginBlock::default_maxstack);

  filter = openrasp_config->Get("plugin.filter", true);
};

const int64_t LogBlock::default_maxburst = 100;
const int64_t LogBlock::default_maxstack = 10;

void LogBlock::update(OpenraspConfig *openrasp_config)
{
  maxburst = openrasp_config->Get("log.maxburst", LogBlock::default_maxburst);
  g_zero_filter(maxburst, LogBlock::default_maxburst);

  maxstack = openrasp_config->Get("log.maxstack", LogBlock::default_maxstack);
  g_zero_filter(maxstack, LogBlock::default_maxstack);
};

const std::string SyslogBlock::default_tag = "OpenRASP";
const int64_t SyslogBlock::default_facility = 1;
const int64_t SyslogBlock::default_connection_timeout = 50;
const int64_t SyslogBlock::default_read_timeout = 10;
const int64_t SyslogBlock::default_reconnect_interval = 300;

void SyslogBlock::update(OpenraspConfig *openrasp_config)
{
  tag = openrasp_config->Get("syslog.tag", SyslogBlock::default_tag);
  regex_filter(tag, "^[0-9a-zA-Z]{1,32}$", SyslogBlock::default_tag);

  url = openrasp_config->Get("syslog.url", std::string(""));
  enable = openrasp_config->Get("syslog.enable", false);

  facility = openrasp_config->Get("syslog.facility", SyslogBlock::default_facility);
  g_zero_filter(facility, SyslogBlock::default_facility);

  connection_timeout = openrasp_config->Get("syslog.connection_timeout", SyslogBlock::default_connection_timeout);
  g_zero_filter(connection_timeout, SyslogBlock::default_connection_timeout);

  read_timeout = openrasp_config->Get("syslog.read_timeout", SyslogBlock::default_read_timeout);
  g_zero_filter(read_timeout, SyslogBlock::default_read_timeout);

  reconnect_interval = openrasp_config->Get("syslog.reconnect_interval", SyslogBlock::default_reconnect_interval);
  g_zero_filter(reconnect_interval, SyslogBlock::default_reconnect_interval);
};

const int64_t BlockBlock::default_status_code = 302;

void BlockBlock::update(OpenraspConfig *openrasp_config)
{
  status_code = openrasp_config->Get("block.status_code", BlockBlock::default_status_code);
  g_zero_filter(status_code, BlockBlock::default_status_code);

  redirect_url = openrasp_config->Get("block.redirect_url", std::string(R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)"));
  content_json = openrasp_config->Get("block.content_json", std::string(R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})"));
  content_xml = openrasp_config->Get("block.content_xml", std::string(R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)"));
  content_html = openrasp_config->Get("block.content_html", std::string(R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)"));
};

void InjectBlock::update(OpenraspConfig *openrasp_config)
{
  urlprefix = openrasp_config->Get("inject.urlprefix", std::string(""));
};

const int64_t BodyBlock::default_maxbytes = 4 * 1024;

void BodyBlock::update(OpenraspConfig *openrasp_config)
{
  maxbytes = openrasp_config->Get("body.maxbytes", BodyBlock::default_maxbytes);
  g_zero_filter(maxbytes, BodyBlock::default_maxbytes);
};

void ClientipBlock::update(OpenraspConfig *openrasp_config)
{
  header = openrasp_config->Get("clientip.header", std::string(""));
};

void SecurityBlock::update(OpenraspConfig *openrasp_config)
{
  enforce_policy = openrasp_config->Get("security.enforce_policy", false);
};

const int64_t SqlBlock::default_slowquery_min_rows = 500;

void SqlBlock::update(OpenraspConfig *openrasp_config)
{
  slowquery.min_rows = openrasp_config->Get("sql.slowquery.min_rows", SqlBlock::default_slowquery_min_rows);
  g_zero_filter(slowquery.min_rows, SqlBlock::default_slowquery_min_rows);
};

const int64_t LruBlock::default_max_size = 1024;

void LruBlock::update(OpenraspConfig *openrasp_config)
{
  max_size = openrasp_config->Get("lru.max_size", LruBlock::default_max_size);
  ge_zero_filter(max_size, LruBlock::default_max_size);
};

const vector<string> CallableBlock::default_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};

void CallableBlock::update(OpenraspConfig *openrasp_config)
{
  blacklist = openrasp_config->GetArray("webshell_callable.blacklist", CallableBlock::default_blacklist);
}

} // namespace openrasp