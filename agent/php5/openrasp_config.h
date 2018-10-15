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
#include <memory>
#include "third_party/rapidjson/document.h"
#include "third_party/cpptoml/cpptoml.h"

namespace openrasp
{
using namespace std;
class OpenraspConfig
{
public:
  // plugin
  int64_t plugin_timeout_ms = 100;
  int64_t plugin_maxstack = 100;
  bool plugin_filter = true;
  // log
  string syslog_server_address;
  int64_t syslog_facility = 1;
  bool syslog_alarm_enable = false;
  int64_t syslog_connection_timeout = 50;
  int64_t syslog_read_timeout = 10;
  int64_t syslog_reconnect_interval = 300;
  int64_t log_maxburst = 100;
  int64_t log_maxstack = 10;
  int64_t log_max_backup = 90;
  // blacklist whitelist
  vector<string> whitelist_command;
  vector<string> whitelist_directory;
  vector<string> whitelist_writeFile;
  vector<string> whitelist_readFile;
  vector<string> whitelist_fileUpload;
  vector<string> whitelist_rename;
  vector<string> whitelist_copy;
  vector<string> whitelist_include;
  vector<string> whitelist_sql;
  vector<string> blacklist_callable = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};
  // block repsonse
  int64_t block_status_code = 302;
  string block_redirect_url = R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)";
  string block_content_json = R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})";
  string block_content_xml = R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)";
  string block_content_html = R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)";
  // others
  string clientip_header;
  string inject_html_urlprefix;
  int64_t body_maxbytes = 4 * 1024;
  int64_t slowquery_min_rows = 500;
  bool enforce_policy = false;

public:
  enum FromType
  {
    kJson,
    kIni
  };

  OpenraspConfig(){};
  OpenraspConfig(string &config, FromType type);
  OpenraspConfig(OpenraspConfig &) = default;
  OpenraspConfig(OpenraspConfig &&) = default;
  OpenraspConfig &operator=(OpenraspConfig &) = default;
  OpenraspConfig &operator=(OpenraspConfig &&) = default;
  bool FromJson(const string &json);
  bool FromIni(const string &ini);
  bool HasError() const { return has_error; };
  string GetErrorMessage() const { return error_message; };

  template <typename T>
  T Get(const string &key, const T &default_value = T()) const
  {
    static_assert(is_same<T, string>::value || is_same<T, int64_t>::value || is_same<T, double>::value || is_same<T, bool>::value,
                  "only support std::string, int64_t, double, bool");
    if (has_error)
    {
      return default_value;
    }
    switch (fromType)
    {
    case FromType::kJson:
      return GetFromJson<T>(key, default_value);
      break;
    case FromType::kIni:
      return GetFromIni<T>(key, default_value);
      break;
    default:
      return default_value;
      break;
    }
  };
  template <typename T>
  vector<T> GetArray(const string &key, const vector<T> &default_value = vector<T>()) const
  {
    static_assert(is_same<T, string>::value || is_same<T, int64_t>::value || is_same<T, double>::value || is_same<T, bool>::value,
                  "only support std::string, int64_t, double, bool");
    if (has_error)
    {
      return default_value;
    }
    switch (fromType)
    {
    case FromType::kJson:
      return GetArrayFromJson<T>(key, default_value);
      break;
    case FromType::kIni:
      return GetArrayFromIni<T>(key, default_value);
      break;
    default:
      return default_value;
      break;
    }
  };

private:
  FromType fromType;
  shared_ptr<rapidjson::Document> jsonObj;
  shared_ptr<cpptoml::table> tomlObj;
  bool has_error = true;
  string error_message = "Uninitialized";

  template <typename T>
  T GetFromJson(const string &key, const T &default_value) const;
  template <typename T>
  vector<T> GetArrayFromJson(const string &key, const vector<T> &default_value) const;

  template <typename T>
  const T &GetFromIni(const string &key, const T &default_value) const { return tomlObj->get_as<T>(key).value_or(default_value); };
  template <typename T>
  const vector<T> &GetArrayFromIni(const string &key, const vector<T> &default_value) const { return tomlObj->get_array_of<T>(key).value_or(default_value); };
};
} // namespace openrasp