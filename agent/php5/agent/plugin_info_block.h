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
#include "openrasp_hook.h"

namespace openrasp
{

class PluginInfoBlock
{
private:
  /*plugin info*/
  static const int plugin_md5_size = 32;
  static const int plugin_name_size = 50;
  static const int plugin_version_size = 50;
  long last_update_time = 0;
  char plugin_md5[PluginInfoBlock::plugin_md5_size + 1] = {0};
  char plugin_name[PluginInfoBlock::plugin_name_size + 1] = {0};
  char plugin_version[PluginInfoBlock::plugin_version_size + 1] = {0};


public:
  /*plugin info*/
  void set_plugin_version(const char *plugin_version);
  const char *get_plugin_version();

  void set_plugin_name(const char *plugin_name);
  const char *get_plugin_name();

  void set_plugin_md5(const char *plugin_md5);
  const char *get_plugin_md5();

  long get_last_update_time();
};

} // namespace openrasp
