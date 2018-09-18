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

#ifndef _OPENRASP_SAFE_SHUTDOWN_MANAGER_H_
#define _OPENRASP_SAFE_SHUTDOWN_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include <memory>

namespace openrasp
{

class ShmManager;

class SafeShutDownManager : public BaseManager
{
public:
  SafeShutDownManager(ShmManager *mm);
  virtual bool startup();
  virtual bool shutdown();
  bool is_master_current_process();
  pid_t get_master_pid();

private:
  pid_t *master_pid = nullptr;
  pid_t init_process_pid;
  pid_t search_fpm_master_pid();
};

extern std::unique_ptr<SafeShutDownManager> ssdm;

} // namespace openrasp

#endif
