/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "openrasp_v8.h"

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

void PluginBlock::update(BaseReader *reader)
{
  timeout.millis = reader->fetch_int64({"plugin.timeout.millis"}, PluginBlock::default_timeout_millis);
  g_zero_filter(timeout.millis, PluginBlock::default_timeout_millis);

  maxstack = reader->fetch_int64({"plugin.maxstack"}, PluginBlock::default_maxstack);
  g_zero_filter(maxstack, PluginBlock::default_maxstack);

  filter = reader->fetch_bool({"plugin.filter"}, true);
};

const int64_t LogBlock::default_maxburst = 100;
const int64_t LogBlock::default_maxstack = 50;

void LogBlock::update(BaseReader *reader)
{
  maxburst = reader->fetch_int64({"log.maxburst"}, LogBlock::default_maxburst);
  g_zero_filter(maxburst, LogBlock::default_maxburst);

  maxstack = reader->fetch_int64({"log.maxstack"}, LogBlock::default_maxstack);
  g_zero_filter(maxstack, LogBlock::default_maxstack);
};

const std::string SyslogBlock::default_tag = "OpenRASP";
const int64_t SyslogBlock::default_facility = 1;
const int64_t SyslogBlock::default_connection_timeout = 50;
const int64_t SyslogBlock::default_read_timeout = 10;
const int64_t SyslogBlock::default_reconnect_interval = 300;

void SyslogBlock::update(BaseReader *reader)
{
  tag = reader->fetch_string({"syslog.tag"}, SyslogBlock::default_tag);
  regex_filter(tag, "^[0-9a-zA-Z]{1,32}$", SyslogBlock::default_tag);

  url = reader->fetch_string({"syslog.url"}, std::string(""));
  enable = reader->fetch_bool({"syslog.enable"}, false);

  facility = reader->fetch_int64({"syslog.facility"}, SyslogBlock::default_facility);
  g_zero_filter(facility, SyslogBlock::default_facility);

  connection_timeout = reader->fetch_int64({"syslog.connection_timeout"}, SyslogBlock::default_connection_timeout);
  g_zero_filter(connection_timeout, SyslogBlock::default_connection_timeout);

  read_timeout = reader->fetch_int64({"syslog.read_timeout"}, SyslogBlock::default_read_timeout);
  g_zero_filter(read_timeout, SyslogBlock::default_read_timeout);

  reconnect_interval = reader->fetch_int64({"syslog.reconnect_interval"}, SyslogBlock::default_reconnect_interval);
  g_zero_filter(reconnect_interval, SyslogBlock::default_reconnect_interval);
};

const int64_t BlockBlock::default_status_code = 302;

void BlockBlock::update(BaseReader *reader)
{
  status_code = reader->fetch_int64({"block.status_code"}, BlockBlock::default_status_code);
  g_zero_filter(status_code, BlockBlock::default_status_code);

  redirect_url = reader->fetch_string({"block.redirect_url"}, std::string(R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)"));
  content_json = reader->fetch_string({"block.content_json"}, std::string(R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})"));
  content_xml = reader->fetch_string({"block.content_xml"}, std::string(R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)"));
  content_html = reader->fetch_string({"block.content_html"}, std::string(R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)"));
};

void InjectBlock::update(BaseReader *reader)
{
  urlprefix = reader->fetch_string({"inject.urlprefix"});
  const auto custom_headers_keys = reader->fetch_object_keys({"inject.custom_headers"});
  headers.clear();
  for (const auto &key : custom_headers_keys)
  {
    const auto &value = reader->fetch_string({"inject.custom_headers", key});
    headers.emplace_back(key + ": " + value);
  }
};

const int64_t BodyBlock::default_maxbytes = 4 * 1024;

void BodyBlock::update(BaseReader *reader)
{
  maxbytes = reader->fetch_int64({"body.maxbytes"}, BodyBlock::default_maxbytes);
  g_zero_filter(maxbytes, BodyBlock::default_maxbytes);
};

void ClientipBlock::update(BaseReader *reader)
{
  header = reader->fetch_string({"clientip.header"}, std::string(""));
};

void SecurityBlock::update(BaseReader *reader)
{
  enforce_policy = reader->fetch_bool({"security.enforce_policy"}, false);
};

const int64_t LruBlock::default_max_size = 1024;

void LruBlock::update(BaseReader *reader)
{
  max_size = reader->fetch_int64({"lru.max_size"}, LruBlock::default_max_size);
  ge_zero_filter(max_size, LruBlock::default_max_size);
};

void DecompileBlock::update(BaseReader *reader)
{
  enable = reader->fetch_bool({"decompile.enable"}, false);
};

const vector<string> CallableBlock::default_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};

void CallableBlock::update()
{
  blacklist = CallableBlock::default_blacklist;
  if (nullptr != OPENRASP_V8_G(isolate))
  {
    extract_callable_blacklist(OPENRASP_V8_G(isolate), blacklist);
  }
}

const int64_t XssBlock::default_min_param_length = 15;
const int64_t XssBlock::default_max_detection_num = 10;
const std::string XssBlock::default_filter_regex = "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]";

void XssBlock::update()
{
  filter_regex = XssBlock::default_filter_regex;
  min_param_length = XssBlock::default_min_param_length;
  max_detection_num = XssBlock::default_max_detection_num;
  if (nullptr != OPENRASP_V8_G(isolate))
  {
    extract_xss_config(OPENRASP_V8_G(isolate), filter_regex, min_param_length, max_detection_num);
  }
}

} // namespace openrasp