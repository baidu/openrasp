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

#include "plugin_update_pkg.h"
#include "agent/shared_config_manager.h"

namespace openrasp
{

const std::string PluginUpdatePackage::snapshot_filename = "snapshot.dat";

PluginUpdatePackage::PluginUpdatePackage(std::string content, std::string version, std::string name, std::string md5)
    : active_plugin(name, content)
{
  plugin_version = version;
  plugin_name = name;
  plugin_md5 = md5;
}

bool PluginUpdatePackage::build_snapshot()
{
  Platform::Get()->Startup();
  Snapshot snapshot(process_globals.plugin_config, {active_plugin}, OpenRASPInfo::PHP_OPENRASP_VERSION, 0);
  Platform::Get()->Shutdown();
  if (!snapshot.IsOk())
  {
    openrasp_error(LEVEL_WARNING, PLUGIN_ERROR, _("Fail to initialize builtin js code, error %s."), strerror(errno));
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
    openrasp_error(LEVEL_WARNING, PLUGIN_ERROR, _("Fail to write snapshot to %s, cuz of %s."),
                   snapshot_abs_path.c_str(), strerror(errno));
  }
  std::map<std::string, std::string> buildin_action_map = CheckTypeTransfer::instance().get_buildin_action_map();
  Platform::Get()->Startup();
  Isolate *isolate = Isolate::New(&snapshot, 0);
  extract_buildin_action(isolate, buildin_action_map);
  std::map<OpenRASPCheckType, OpenRASPActionType> type_action_map;
  for (auto iter = buildin_action_map.begin(); iter != buildin_action_map.end(); iter++)
  {
    type_action_map.insert({CheckTypeTransfer::instance().name_to_type(iter->first), string_to_action(iter->second)});
  }
  openrasp::scm->set_buildin_check_action(type_action_map);
  openrasp::scm->set_mysql_error_codes(extract_int64_array(isolate, "RASP.algorithmConfig.sql_exception.mysql.error_code", SharedConfigBlock::MYSQL_ERROR_CODE_MAX_SIZE));
  openrasp::scm->set_sqlite_error_codes(extract_int64_array(isolate, "RASP.algorithmConfig.sql_exception.sqlite.error_code", SharedConfigBlock::SQLITE_ERROR_CODE_MAX_SIZE));
  openrasp::scm->build_pg_error_array(isolate);
  openrasp::scm->build_env_key_array(isolate);
  isolate->Dispose();
  Platform::Get()->Shutdown();
  return build_successful;
}

std::string PluginUpdatePackage::get_version() const
{
  return plugin_version;
}

std::string PluginUpdatePackage::get_name() const
{
  return plugin_name;
}

std::string PluginUpdatePackage::get_md5() const
{
  return plugin_md5;
}

} // namespace openrasp
