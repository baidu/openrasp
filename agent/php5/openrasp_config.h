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

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>

namespace openrasp
{
using namespace std;
class OpenraspConfig
{
public:
  // plugin
  uint32_t plugin_timeout_ms = 100;
  uint32_t plugin_maxstack = 100;
  bool plugin_filter = true;
  // log
  string syslog_server_address;
  uint32_t syslog_facility;
  bool syslog_alarm_enable = false;
  uint32_t syslog_connection_timeout = 50;
  uint32_t syslog_read_timeout = 10;
  uint32_t syslog_connection_retry_interval = 200;
  uint32_t log_maxburst = 100;
  uint32_t log_maxstack = 10;
  uint32_t log_max_backup = 90;
  uint32_t log_push_interval = 10;
  // blacklist whitelist
  unordered_set<string> hooks_ignore;
  unordered_set<string> callable_blacklists;
  // block repsonse
  uint32_t block_status_code = 302;
  string block_redirect_url;
  string block_content_json;
  string block_content_xml;
  string block_content_html;
  // others
  string clientip_header;
  uint32_t body_maxbytes = 4 * 1024;
  string inject_html_urlprefix;
  uint32_t slowquery_min_rows = 500;
  bool enforce_policy = false;

public:
  enum FromType
  {
    json,
    ini
  };
  OpenraspConfig(){};
  OpenraspConfig(string &config, FromType type);
  OpenraspConfig(OpenraspConfig &) = default;
  OpenraspConfig(OpenraspConfig &&) = default;
  OpenraspConfig &operator=(OpenraspConfig &) = default;
  OpenraspConfig &operator=(OpenraspConfig &&) = default;
  void FromJson(string &json);
  void FromIni(string &ini);
};
} // namespace openrasp