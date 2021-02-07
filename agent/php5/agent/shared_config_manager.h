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

#ifndef _OPENRASP_SHARED_CONFIG_MANAGER_H_
#define _OPENRASP_SHARED_CONFIG_MANAGER_H_

#include "openrasp.h"
#include "base_manager.h"
#include <memory>
#include <map>
#include "utils/read_write_lock.h"
#include "shared_config_block.h"
#include "utils/base_reader.h"

namespace openrasp
{

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
  bool set_debug_level(BaseReader *br);

  dat_value get_check_type_white_bit_mask(std::string url);
  bool build_check_type_white_array(BaseReader *br);

  bool build_weak_password_array(BaseReader *br);
  bool is_password_weak(std::string password);

  bool build_pg_error_array(std::vector<std::string> &pg_errors);
  bool build_pg_error_array(Isolate *isolate);
  bool pg_error_filtered(std::string error);

  bool build_env_key_array(std::vector<std::string> &env_keys);
  bool build_env_key_array(Isolate *isolate);
  bool filter_env_key(const std::string &env);

  std::string get_rasp_id() const;

  bool set_buildin_check_action(std::map<OpenRASPCheckType, OpenRASPActionType> buildin_action_map);
  OpenRASPActionType get_buildin_check_action(OpenRASPCheckType check_type);

  void set_mysql_error_codes(std::vector<int64_t> error_codes);
  bool mysql_error_code_exist(int64_t err_code);

  void set_sqlite_error_codes(std::vector<int64_t> error_codes);
  bool sqlite_error_code_exist(int64_t err_code);

private:
  int meta_size;
  ReadWriteLock *rwlock;
  SharedConfigBlock *shared_config_block;
  std::string rasp_id;

  bool build_check_type_white_array(std::map<std::string, dat_value> &url_mask_map);
  bool build_check_type_white_array(std::map<std::string, std::vector<std::string>> &url_type_map);

  bool build_weak_password_array(std::vector<std::string> &weak_passwords);

  bool set_debug_level(long debug_level);

  bool write_check_type_white_array_to_shm(const void *source, size_t num);
  bool write_weak_password_array_to_shm(const void *source, size_t num);
  bool write_pg_error_array_to_shm(const void *source, size_t num);
  bool write_env_key_array_to_shm(const void *source, size_t num);
  bool build_rasp_id();
};

extern std::unique_ptr<SharedConfigManager> scm;

} // namespace openrasp

#endif
