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

#ifndef _OPENRASP_SHARED_LOG_MANAGER_H_
#define _OPENRASP_SHARED_LOG_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include <memory>
#include <map>
#include "utils/read_write_lock.h"
#include "shared_log_block.h"

namespace openrasp
{

class SharedLogManager : public BaseManager
{
public:
  SharedLogManager();
  virtual ~SharedLogManager();
  virtual bool startup();
  virtual bool shutdown();

  bool log_exist(long timestamp, ulong log_hash);
  bool log_update(long timestamp, ulong log_hash);
private:
  int meta_size;
  ReadWriteLock *rwlock;
  SharedLogBlock *shared_log_block;
};

} // namespace openrasp

#endif
