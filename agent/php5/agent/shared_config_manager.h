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

#ifndef _OPENRASP_SHARED_CONFIG_MANAGER_H_
#define _OPENRASP_SHARED_CONFIG_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include <memory>
#include <map>
#include "utils/ReadWriteLock.h"
#include "shared_config_block.h"

namespace openrasp
{

class SharedConfigManager : public BaseManager
{
public:
  SharedConfigManager(ShmManager *mm);
  virtual bool startup();
  virtual bool shutdown();
  int get_check_type_white_bit_mask(std::string url);
  bool build_check_type_white_array(std::map<std::string, int> &url_mask_map);

private:
  VariableConfigBlock *variable_config_block;
  ReadWriteLock *rwlock;
  bool write_check_type_white_array_to_shm(const void *source, size_t num);
};

extern std::unique_ptr<SharedConfigManager> vcm;

} // namespace openrasp

#endif
