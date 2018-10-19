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
  struct
  {
    struct
    {
      int64_t millis = 100;
    } timeout;
    int64_t maxstack = 100;
    bool filter = true;
  } plugin;
  // log
  struct
  {
    int64_t maxburst = 100;
    int64_t maxstack = 10;
    int64_t maxbackup = 30;
  } log;
  string syslog_server_address;
  int64_t syslog_facility = 1;
  bool syslog_alarm_enable = false;
  int64_t syslog_connection_timeout = 50;
  int64_t syslog_read_timeout = 10;
  int64_t syslog_reconnect_interval = 300;
  // blacklist whitelist
  struct
  {
    struct
    {
      vector<string> callable;
      vector<string> command;
      vector<string> directory;
      vector<string> readFile;
      vector<string> writeFile;
      vector<string> copy;
      vector<string> rename;
      vector<string> fileUpload;
      vector<string> include;
      vector<string> dbConnection;
      vector<string> sql;
      vector<string> sqlSlowQuery;
      vector<string> sqlPrepared;
      vector<string> ssrf;
      vector<string> wenshell_eval;
      vector<string> wenshell_command;
      vector<string> webshell_file_put_contents;
      bool All = false;
    } white;
  } hook;
  vector<string> callable_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};
  // block repsonse
  struct
  {
    int64_t status_code = 302;
    string redirect_url = R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)";
    string content_json = R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})";
    string content_xml = R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)";
    string content_html = R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)";
  } block;
  // others
  struct
  {
    string urlprefix;
  } inject;
  struct
  {
    int64_t maxbytes = 4 * 1024;
  } body;
  struct
  {
    string header;
  } clientip;
  struct
  {
    bool enforce_policy = false;
  } security;
  struct
  {
    struct
    {
      int64_t min_rows = 500;
    } slowquery;
  } sql;
  int64_t slowquery_min_rows = 500;
  int64_t lru_cache_max_size = 1000;

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
  const T GetFromIni(const string &key, const T &default_value) const
  {
    size_t pos = key.find_last_of(".");
    if (pos != string::npos && pos != key.size() - 1 && tomlObj->contains_qualified(key))
    {
      auto inner = tomlObj->get_table_qualified(key.substr(0, pos));
      return inner->get_as<T>(key.substr(pos + 1)).value_or(default_value);
    }
    return tomlObj->get_as<T>(key).value_or(default_value);
  };

  template <typename T>
  const vector<T> GetArrayFromIni(const string &key, const vector<T> &default_value) const
  {
    size_t pos = key.find_last_of(".");
    if (pos != string::npos && pos != key.size() - 1 && tomlObj->contains_qualified(key))
    {
      auto inner = tomlObj->get_table_qualified(key.substr(0, pos));
      return inner->get_array_of<T>(key.substr(pos + 1)).value_or(default_value);
    }
    return tomlObj->get_array_of<T>(key).value_or(default_value);
  };
};
} // namespace openrasp