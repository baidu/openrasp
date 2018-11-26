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

#pragma once

#include "openrasp.h"
#include "openrasp_hook.h"
#include "utils/DoubleArrayTrie.h"
#include <string>

namespace openrasp
{
#define WRITE_ARRAY_MAX_LENGTH (CHECK_TYPE_NR_ITEMS * 10 * 200 + 128 * 2)

class SharedConfigBlock
{
public:
  inline openrasp::DoubleArrayTrie::unit_t *get_check_type_white_array()
  {
    return check_type_white_array;
  }

  inline size_t get_white_array_size()
  {
    return white_array_size;
  }

  inline bool reset_white_array(const void *source, size_t num)
  {
    if (num > WRITE_ARRAY_MAX_LENGTH)
    {
      return false;
    }
    memset(&check_type_white_array, 0, sizeof(check_type_white_array));
    memcpy((void *)&check_type_white_array, source, num);
    white_array_size = num;
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
    switch (check_type)
    {
    case CALLABLE:
      callable_action = action_type;
      break;
    case WEBSHELL_EVAL:
      webshell_eval_action = action_type;
      break;
    case WEBSHELL_COMMAND:
      webshell_command_action = action_type;
      break;
    case WEBSHELL_FILE_PUT_CONTENTS:
      webshell_file_put_contents_action = action_type;
      break;
    default:
      break;
    }
  }

  inline OpenRASPActionType get_check_type_action(OpenRASPCheckType check_type) const
  {
    OpenRASPActionType action_type = AC_IGNORE;
    switch (check_type)
    {
    case CALLABLE:
      action_type = callable_action;
      break;
    case WEBSHELL_EVAL:
      action_type = webshell_eval_action;
      break;
    case WEBSHELL_COMMAND:
      action_type = webshell_command_action;
      break;
    case WEBSHELL_FILE_PUT_CONTENTS:
      action_type = webshell_file_put_contents_action;
      break;
    default:
      break;
    }
    return action_type;
  }

private:
  long config_update_time = 0;
  long log_max_backup = 0;
  long debug_level = 0;
  size_t white_array_size;
  OpenRASPActionType callable_action;
  OpenRASPActionType webshell_eval_action;
  OpenRASPActionType webshell_command_action;
  OpenRASPActionType webshell_file_put_contents_action;
  openrasp::DoubleArrayTrie::unit_t check_type_white_array[WRITE_ARRAY_MAX_LENGTH + 1];
};

} // namespace openrasp
