/*
 * Copyright 2017-2021 Baidu Inc.
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
#include "utils/validator.h"
#include "openrasp_v8.h"

namespace openrasp
{
const int64_t PluginBlock::default_timeout_millis = 100;
const int64_t PluginBlock::default_maxstack = 100;

void PluginBlock::update(BaseReader *reader)
{
  timeout.millis = reader->fetch_int64({"plugin.timeout.millis"}, PluginBlock::default_timeout_millis, openrasp::g_zero_int64);
  maxstack = reader->fetch_int64({"plugin.maxstack"}, PluginBlock::default_maxstack, openrasp::ge_zero_int64);
  filter = reader->fetch_bool({"plugin.filter"}, true);
};

const int64_t LogBlock::default_maxburst = 100;

void LogBlock::update(BaseReader *reader)
{
  maxburst = reader->fetch_int64({"log.maxburst"}, LogBlock::default_maxburst, openrasp::ge_zero_int64);
};

const std::string SyslogBlock::default_tag = "OpenRASP";
const int64_t SyslogBlock::default_facility = 1;
const int64_t SyslogBlock::default_connection_timeout = 50;
const int64_t SyslogBlock::default_read_timeout = 10;
const int64_t SyslogBlock::default_reconnect_interval = 300;

void SyslogBlock::update(BaseReader *reader)
{
  tag = reader->fetch_string({"syslog.tag"}, SyslogBlock::default_tag,
                             [](const std::string &value) {
                               return openrasp::regex_string(value, "^[0-9a-zA-Z]{1,32}$", "should be number and alphabeta, and length between 1 and 32");
                             });
  url = reader->fetch_string({"syslog.url"}, std::string(""));
  enable = reader->fetch_bool({"syslog.enable"}, false);
  facility = reader->fetch_int64({"syslog.facility"}, SyslogBlock::default_facility, openrasp::ge_zero_int64);
  connection_timeout = reader->fetch_int64({"syslog.connection_timeout"}, SyslogBlock::default_connection_timeout, openrasp::g_zero_int64);
  read_timeout = reader->fetch_int64({"syslog.read_timeout"}, SyslogBlock::default_read_timeout, openrasp::g_zero_int64);
  reconnect_interval = reader->fetch_int64({"syslog.reconnect_interval"}, SyslogBlock::default_reconnect_interval, openrasp::g_zero_int64);
};

const int64_t BlockBlock::default_status_code = 302;

void BlockBlock::update(BaseReader *reader)
{
  status_code = reader->fetch_int64({"block.status_code"}, BlockBlock::default_status_code, openrasp::ge_zero_int64);
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
    if (!key.empty() && key.length() <= 200)
    {
      const std::string value = reader->fetch_string({"inject.custom_headers", key});
      if (!value.empty() && value.length() <= 200)
      {
        headers.emplace_back(key + ": " + value);
      }
    }
  }
};

const int64_t BodyBlock::default_maxbytes = 4 * 1024;

void BodyBlock::update(BaseReader *reader)
{
  maxbytes = reader->fetch_int64({"body.maxbytes"}, BodyBlock::default_maxbytes, openrasp::ge_zero_int64);
};

void ClientipBlock::update(BaseReader *reader)
{
  header = reader->fetch_string({"clientip.header"}, std::string("ClientIP"));
};

const int64_t LruBlock::default_max_size = 1024;

void LruBlock::update(BaseReader *reader)
{
  max_size = reader->fetch_int64({"lru.max_size"}, LruBlock::default_max_size, openrasp::ge_zero_int64);
};

void DecompileBlock::update(BaseReader *reader)
{
  enable = reader->fetch_bool({"decompile.enable"}, false);
};

void ResponseBlock::update(BaseReader *reader)
{
  sampler_interval = reader->fetch_int64({"response.sampler_interval"}, 60,
                                         [](int64_t value) {
                                           return openrasp::limit_int64(value, 60, true);
                                         });
  sampler_burst = reader->fetch_int64({"response.sampler_burst"}, 5);
};

} // namespace openrasp