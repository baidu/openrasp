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

#include "base_reader.h"
#include <algorithm>

namespace openrasp
{

const std::set<std::string> BaseReader::valid_keys = {
    "plugin.timeout.millis",
    "plugin.maxstack",
    "plugin.filter",
    "log.maxburst",
    "log.maxstack",
    "log.maxbackup",
    "syslog.enable",
    "syslog.tag",
    "syslog.url",
    "syslog.facility",
    "syslog.connection_timeout",
    "syslog.read_timeout",
    "syslog.reconnect_interval",
    "block.status_code",
    "block.redirect_url",
    "block.content_json",
    "block.content_xml",
    "block.content_html",
    "inject.urlprefix",
    "inject.custom_headers",
    "body.maxbytes",
    "clientip.header",
    "security.enforce_policy",
    "security.weak_passwords",
    "lru.max_size",
    "debug.level",
    "hook.white",
    "ognl.expression.minlength", //used for java only
    "dependency_check.interval",
    "webroot_scan.scan_limit",
    "webroot_scan.interval",
    "decompile.enable"};

std::string BaseReader::check_config_key(const std::vector<std::string> &keys)
{
  std::vector<std::string> found_keys = fetch_object_keys(keys);
  for (auto &key : found_keys)
  {
    auto found = valid_keys.find(key);
    if (found == valid_keys.end())
    {
      return key;
    }
  }
  return "";
}

} // namespace openrasp
