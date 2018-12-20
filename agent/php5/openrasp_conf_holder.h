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
#include "openrasp_config.h"
#include "openrasp_config_block.h"

namespace openrasp
{
using namespace std;

class ConfigHolder
{
public:
  ConfigHolder(){};
  ConfigHolder(string &config, OpenraspConfig::FromType type);
  bool init(string &config, OpenraspConfig::FromType type);
  bool update(OpenraspConfig *openrasp_config);
  long GetLatestUpdateTime() const;
  void SetLatestUpdateTime(long latestUpdateTime);

public:
  PluginBlock plugin;
  LogBlock log;
  SyslogBlock syslog;
  BlockBlock block;
  InjectBlock inject;
  BodyBlock body;
  ClientipBlock clientip;
  SecurityBlock security;
  SqlBlock sql;
  LruBlock lru;
  CallableBlock webshell_callable;

private:
  long latestUpdateTime = 0;
};
} // namespace openrasp