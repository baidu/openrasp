/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "utils/BaseReader.h"

namespace openrasp
{
#define ROUNDUP(x, n) (((x) + ((n)-1)) & (~((n)-1)))

class SharedConfigManager : public BaseManager
{
public:
  SharedConfigManager();
  virtual ~SharedConfigManager();
  virtual bool startup();
  virtual bool shutdown();

  long get_config_last_update();
  bool set_config_last_update(long config_update_timestamp);

  long get_log_max_backup();
  bool set_log_max_backup(long log_max_backup);

  long get_debug_level();
  bool set_debug_level(long debug_level);

  int get_check_type_white_bit_mask(std::string url);
  bool build_check_type_white_array(std::map<std::string, int> &url_mask_map);
  bool build_check_type_white_array(std::map<std::string, std::vector<std::string>> &url_type_map);

  std::string get_rasp_id() const;
  std::string get_hostname() const;

  bool set_buildin_check_action(std::map<OpenRASPCheckType, OpenRASPActionType> buildin_action_map);
  OpenRASPActionType get_buildin_check_action(OpenRASPCheckType check_type);

private:
  int meta_size;
  ReadWriteLock *rwlock;
  SharedConfigBlock *shared_config_block;
  std::string rasp_id;
  std::string hostname;

private:
  bool write_check_type_white_array_to_shm(const void *source, size_t num);
  bool build_hostname();
  bool build_rasp_id();
};

extern std::unique_ptr<SharedConfigManager> scm;

} // namespace openrasp

#endif
