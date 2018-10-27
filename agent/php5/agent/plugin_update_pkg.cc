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

#include "plugin_update_pkg.h"

namespace openrasp
{

const std::string PluginUpdatePackage::snapshot_filename = "snapshot.dat";

PluginUpdatePackage::PluginUpdatePackage(std::string content, std::string version)
    : active_plugin(version, content)
{
  plugin_version = version;
}

void PluginUpdatePackage::set_algorithm(std::string algorithm)
{
  algorithm_config = "RASP.algorithmConfig=" + algorithm;
}

bool PluginUpdatePackage::build_snapshot()
{
  Platform::Initialize();
  Snapshot snapshot(algorithm_config, {active_plugin});
  Platform::Shutdown();
  if (!snapshot.IsOk())
  {
    openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to generate snapshot."));
    return false;
  }
  std::string snapshot_abs_path = std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + PluginUpdatePackage::snapshot_filename;
#ifndef _WIN32
  mode_t oldmask = umask(0);
#endif
  bool build_successful = snapshot.Save(snapshot_abs_path);
#ifndef _WIN32
  umask(oldmask);
#endif
  if (!build_successful)
  {
    openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to write snapshot to %s."), snapshot_abs_path.c_str());
  }
  return build_successful;
}

std::string PluginUpdatePackage::get_version() const
{
  return plugin_version;
}

} // namespace openrasp
