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

extern "C"
{
#include <stdio.h>
}
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "plugin_info_block.h"

namespace openrasp
{
/*plugin info*/
void PluginInfoBlock::set_plugin_version(const char *plugin_version)
{
    strncpy(this->plugin_version, plugin_version, PluginInfoBlock::plugin_version_size);
    last_update_time = (long)time(nullptr);
}

const char *PluginInfoBlock::get_plugin_version()
{
    return plugin_version;
}

void PluginInfoBlock::set_plugin_name(const char *plugin_name)
{
    strncpy(this->plugin_name, plugin_name, PluginInfoBlock::plugin_name_size);
}

const char *PluginInfoBlock::get_plugin_name()
{
    return plugin_name;
}

void PluginInfoBlock::set_plugin_md5(const char *plugin_md5)
{
    strncpy(this->plugin_md5, plugin_md5, PluginInfoBlock::plugin_md5_size);
}

const char *PluginInfoBlock::get_plugin_md5()
{
    return plugin_md5;
}

long PluginInfoBlock::get_last_update_time()
{
    return last_update_time;
}
} // namespace openrasp
