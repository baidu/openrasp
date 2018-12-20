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

#include "openrasp.h"
#include "openrasp_conf_holder.h"

namespace openrasp
{
ConfigHolder::ConfigHolder(string &config, OpenraspConfig::FromType type)
{
  init(config, type);
}

bool ConfigHolder::init(string &config, OpenraspConfig::FromType type)
{
  OpenraspConfig openrasp_config(config, type);
  if (openrasp_config.HasError())
  {
    return false;
  }
  return update(&openrasp_config);
}

bool ConfigHolder::update(OpenraspConfig *openrasp_config)
{
  if (!openrasp_config)
  {
    return false;
  }
  plugin.update(openrasp_config);
  log.update(openrasp_config);
  syslog.update(openrasp_config);
  block.update(openrasp_config);
  inject.update(openrasp_config);
  body.update(openrasp_config);
  clientip.update(openrasp_config);
  security.update(openrasp_config);
  sql.update(openrasp_config);
  lru.update(openrasp_config);
  webshell_callable.update(openrasp_config);
  return true;
}

long ConfigHolder::GetLatestUpdateTime() const
{
  return latestUpdateTime;
}

void ConfigHolder::SetLatestUpdateTime(long latestUpdateTime)
{
  this->latestUpdateTime = latestUpdateTime;
}

} // namespace openrasp