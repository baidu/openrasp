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

#include "openrasp.h"
#include "openrasp_conf_holder.h"

namespace openrasp
{

bool ConfigHolder::update(BaseReader *reader)
{
  if (!reader || reader->has_error())
  {
    return false;
  }
  plugin.update(reader);
  log.update(reader);
  syslog.update(reader);
  block.update(reader);
  inject.update(reader);
  body.update(reader);
  clientip.update(reader);
  lru.update(reader);
  decompile.update(reader);
  response.update(reader);
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
