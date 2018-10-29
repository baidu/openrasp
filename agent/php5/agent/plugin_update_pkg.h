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

#ifndef _PLUGIN_CONTAINER_H_
#define _PLUGIN_CONTAINER_H_

#include "openrasp_config.h"
#include "openrasp_agent.h"

namespace openrasp
{
class PluginUpdatePackage
{
private:
  static const std::string snapshot_filename;

private:
  PluginFile active_plugin;
  std::string plugin_version;
  std::string algorithm_config;

public:
  PluginUpdatePackage(std::string content, std::string version);
  void set_algorithm(std::string algorithm);
  bool build_snapshot();
  std::string get_version() const;
};
} // namespace openrasp
#endif