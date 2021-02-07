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

#include "openrasp.h"
#include "openrasp_hook.h"
#include "utils/double_array_trie.h"
#include <string>

namespace openrasp
{
class SharedConfigBlock
{
public:
  static const int WHITE_ARRAY_MAX_SIZE = (200 * 200 * (DoubleArrayTrie::unit_size()) * 2);
  static const int WEAK_PASSWORD_ARRAY_MAX_SIZE = (200 * 16 * (DoubleArrayTrie::unit_size()) * 2);
  static const int PG_ERROR_ARRAY_MAX_SIZE = (200 * 5 * (DoubleArrayTrie::unit_size()) * 2);
  static const int ENV_KEY_ARRAY_MAX_SIZE = (200 * 50 * (DoubleArrayTrie::unit_size()) * 2);
  static const int MYSQL_ERROR_CODE_MAX_SIZE = 100;
  static const int PGSQL_ERROR_CODE_MAX_SIZE = 100;
  static const int SQLITE_ERROR_CODE_MAX_SIZE = 100;
  static const int WEBSHELL_ENV_KEY_MAX_SIZE = 200;

  inline char *get_check_type_white_array()
  {
    return check_type_white_array;
  }

  inline size_t get_white_array_size()
  {
    return white_array_size;
  }

  inline bool reset_white_array(const void *source, size_t num)
  {
    if (num > WHITE_ARRAY_MAX_SIZE)
    {
      return false;
    }
    memset(&check_type_white_array, 0, sizeof(check_type_white_array));
    memcpy((void *)&check_type_white_array, source, num);
    white_array_size = num;
    return true;
  }

  inline char *get_weak_password_array()
  {
    return weak_password_array;
  }

  inline size_t get_weak_password_array_size()
  {
    return weak_password_array_size;
  }

  inline bool reset_weak_password_array(const void *source, size_t num)
  {
    memset(&weak_password_array, 0, sizeof(weak_password_array));
    if (num > WEAK_PASSWORD_ARRAY_MAX_SIZE)
    {
      return false;
    }
    memcpy((void *)&weak_password_array, source, num);
    weak_password_array_size = num;
    return true;
  }

  inline long get_config_update_time()
  {
    return config_update_time;
  }

  inline void set_config_update_time(long config_update_time)
  {
    this->config_update_time = config_update_time;
  }

  inline long get_log_max_backup()
  {
    return log_max_backup;
  }

  inline void set_log_max_backup(long log_max_backup)
  {
    this->log_max_backup = log_max_backup;
  }

  inline long get_debug_level()
  {
    return debug_level;
  }

  inline void set_debug_level(long debug_level)
  {
    this->debug_level = debug_level;
  }

  inline void set_check_type_action(OpenRASPCheckType check_type, OpenRASPActionType action_type)
  {
    if (check_type > INVALID_TYPE && check_type < ALL_TYPE)
    {
      actions[check_type] = action_type;
    }
  }

  inline OpenRASPActionType get_check_type_action(OpenRASPCheckType check_type) const
  {
    OpenRASPActionType action_type = AC_IGNORE;
    if (check_type > INVALID_TYPE && check_type < ALL_TYPE)
    {
      action_type = actions[check_type];
    }
    return action_type;
  }

  inline void set_mysql_error_codes(std::vector<int64_t> error_codes)
  {
    size_t err_size = error_codes.size();
    if (err_size >= 0 && err_size <= MYSQL_ERROR_CODE_MAX_SIZE)
    {
      for (int i = 0; i < err_size; ++i)
      {
        mysql_error_codes[i] = error_codes[i];
      }
      mysql_error_codes_size = err_size;
    }
    else
    {
      mysql_error_codes_size = 0;
    }
  }

  inline bool mysql_error_code_exist(int64_t err_code) const
  {
    for (int i = 0; i < mysql_error_codes_size; ++i)
    {
      if (mysql_error_codes[i] == err_code)
      {
        return true;
      }
    }
    return false;
  }

  inline void set_sqlite_error_codes(std::vector<int64_t> error_codes)
  {
    size_t err_size = error_codes.size();
    if (err_size >= 0 && err_size <= SQLITE_ERROR_CODE_MAX_SIZE)
    {
      for (int i = 0; i < err_size; ++i)
      {
        sqlite_error_codes[i] = error_codes[i];
      }
      sqlite_error_codes_size = err_size;
    }
    else
    {
      sqlite_error_codes_size = 0;
    }
  }

  inline bool sqlite_error_code_exist(int64_t err_code) const
  {
    for (int i = 0; i < sqlite_error_codes_size; ++i)
    {
      if (sqlite_error_codes[i] == err_code)
      {
        return true;
      }
    }
    return false;
  }

  inline char *get_pg_error_array()
  {
    return pg_error_array;
  }

  inline size_t get_pg_error_array_size()
  {
    return pg_error_array_size;
  }

  inline bool reset_pg_error_array(const void *source, size_t num)
  {
    if (num > PG_ERROR_ARRAY_MAX_SIZE)
    {
      return false;
    }
    memset(&pg_error_array, 0, sizeof(pg_error_array));
    memcpy((void *)&pg_error_array, source, num);
    pg_error_array_size = num;
    return true;
  }

  /* webshell_env*/
  inline char *get_env_key_array()
  {
    return env_key_array;
  }

  inline size_t get_env_key_array_size()
  {
    return env_key_array_size;
  }

  inline bool reset_env_key_array(const void *source, size_t num)
  {
    if (num > ENV_KEY_ARRAY_MAX_SIZE)
    {
      return false;
    }
    memset(&env_key_array, 0, sizeof(env_key_array));
    memcpy((void *)&env_key_array, source, num);
    env_key_array_size = num;
    return true;
  }

private:
  long config_update_time = 0;
  long log_max_backup = 0;
  long debug_level = 0;
  OpenRASPActionType actions[ALL_TYPE] = {AC_IGNORE};

  size_t white_array_size;
  char check_type_white_array[WHITE_ARRAY_MAX_SIZE + 1];

  size_t weak_password_array_size;
  char weak_password_array[WEAK_PASSWORD_ARRAY_MAX_SIZE + 1];

  size_t pg_error_array_size;
  char pg_error_array[PG_ERROR_ARRAY_MAX_SIZE + 1];

  size_t env_key_array_size;
  char env_key_array[ENV_KEY_ARRAY_MAX_SIZE + 1];

  int mysql_error_codes_size = 0;
  long mysql_error_codes[MYSQL_ERROR_CODE_MAX_SIZE] = {0};

  int sqlite_error_codes_size = 0;
  long sqlite_error_codes[SQLITE_ERROR_CODE_MAX_SIZE] = {0};
};

} // namespace openrasp
