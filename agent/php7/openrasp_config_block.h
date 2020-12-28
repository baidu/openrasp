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

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <memory>
#include "utils/base_reader.h"
#include "php/header.h"

namespace openrasp
{
using namespace std;

void g_zero_filter(int64_t &value, const int64_t &dafault);
void ge_zero_filter(int64_t &value, const int64_t &dafault);
void regex_filter(string &value, const string &regex, const string &dafault);

// plugin
class PluginBlock
{
public:
  const static int64_t default_timeout_millis;
  const static int64_t default_maxstack;
  struct
  {
    int64_t millis = 100;
  } timeout;
  int64_t maxstack = 100;
  bool filter = true;
  void update(BaseReader *reader);
};

// log
class LogBlock
{
public:
  const static int64_t default_maxburst;
  int64_t maxburst = 100;
  void update(BaseReader *reader);
};

class SyslogBlock
{
public:
  const static std::string default_tag;
  const static int64_t default_facility;
  const static int64_t default_connection_timeout;
  const static int64_t default_read_timeout;
  const static int64_t default_reconnect_interval;
  string tag = "OpenRASP";
  string url;
  int64_t facility = 1;
  bool enable = false;
  int64_t connection_timeout = 50;
  int64_t read_timeout = 10;
  int64_t reconnect_interval = 300;
  void update(BaseReader *reader);
};

// block repsonse
class BlockBlock
{
public:
  const static int64_t default_status_code;
  int64_t status_code = 302;
  string redirect_url = R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)";
  string content_json = R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})";
  string content_xml = R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)";
  string content_html = R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)";
  void update(BaseReader *reader);
};
// others
class InjectBlock
{
public:
  string urlprefix;
  vector<string> headers;
  void update(BaseReader *reader);
};

class BodyBlock
{
public:
  const static int64_t default_maxbytes;
  int64_t maxbytes = 4 * 1024;
  void update(BaseReader *reader);
};

class ClientipBlock
{
public:
  string header;
  void update(BaseReader *reader);
};

class LruBlock
{
public:
  const static int64_t default_max_size;
  int64_t max_size = 1024;
  void update(BaseReader *reader);
};

class DecompileBlock
{
public:
  bool enable = false;
  void update(BaseReader *reader);
};

class ResponseBlock
{
public:
  int sampler_interval;
  int sampler_burst;
  void update(BaseReader *reader);
};

} // namespace openrasp