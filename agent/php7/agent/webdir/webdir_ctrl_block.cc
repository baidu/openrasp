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
#include "webdir_ctrl_block.h"

namespace openrasp
{
/*webdir block*/

const long WebdirCtrlBlock::default_scan_limit = 100l;
const std::string WebdirCtrlBlock::default_scan_regex = "\\.(git|svn|tar|gz|rar|zip|sql|log)$";

void WebdirCtrlBlock::set_webdir_scan_regex(const char *webdir_scan_regex)
{
  strncpy(this->webdir_scan_regex, webdir_scan_regex, WebdirCtrlBlock::webdir_scan_regex_size);
}

const char *WebdirCtrlBlock::get_webdir_scan_regex()
{
  return webdir_scan_regex;
}

int WebdirCtrlBlock::get_webroot_count()
{
  return webroot_count;
}
void WebdirCtrlBlock::set_webroot_count(int webroot_count)
{
  if (webroot_count >= 0 && webroot_count < WebdirCtrlBlock::webroot_max_size)
  {
    this->webroot_count = webroot_count;
  }
}

int WebdirCtrlBlock::get_dependency_interval()
{
  return dependency_interval;
}

void WebdirCtrlBlock::set_dependency_interval(int dependency_interval)
{
  this->dependency_interval = (dependency_interval >= WebdirCtrlBlock::agent_min_interval &&
                               dependency_interval <= WebdirCtrlBlock::agent_max_interval)
                                  ? dependency_interval
                                  : WebdirCtrlBlock::default_dependency_interval;
}

int WebdirCtrlBlock::get_webdir_scan_interval()
{
  return webdir_scan_interval;
}

void WebdirCtrlBlock::set_webdir_scan_interval(int webdir_scan_interval)
{
  this->webdir_scan_interval = (webdir_scan_interval >= WebdirCtrlBlock::agent_min_interval &&
                                webdir_scan_interval <= WebdirCtrlBlock::agent_max_interval)
                                   ? webdir_scan_interval
                                   : WebdirCtrlBlock::default_webdir_scan_interval;
}

long WebdirCtrlBlock::get_scan_limit()
{
  return scan_limit;
}

void WebdirCtrlBlock::set_scan_limit(long scan_limit)
{
  this->scan_limit = (scan_limit >= 0) ? scan_limit : WebdirCtrlBlock::default_scan_limit;
}

bool WebdirCtrlBlock::webroot_found(ulong hash)
{
  int size = MIN(this->webroot_count, WebdirCtrlBlock::webroot_max_size);
  for (int i = size - 1; i > 0; --i)
  {
    if (this->webroot_hash[i] == hash)
    {
      return true;
    }
  }
  return false;
}
void WebdirCtrlBlock::set_webroot_hash(int index, ulong hash)
{
  if (index >= 0 && index < WebdirCtrlBlock::webroot_max_size)
  {
    this->webroot_hash[index] = hash;
  }
}

void WebdirCtrlBlock::set_webroot_path(const char *webroot_path)
{
  strncpy(this->webroot_path, webroot_path, MAXPATHLEN - 1);
}
const char *WebdirCtrlBlock::get_webroot_path()
{
  return webroot_path;
}

} // namespace openrasp
