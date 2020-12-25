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
#include "utils/time.h"
#include <string>

namespace openrasp
{
#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif

class SharedLogBlock
{
public:
  inline void set_size(size_t size)
  {
    this->size = size;
  }

  inline void update_date(long timestamp)
  {
    long offset = fetch_time_offset();
    this->date = (timestamp + offset) / SharedLogBlock::day_seconds;
    set_size(0);
  }

  inline int compare_date(long timestamp)
  {
    return timestamp / SharedLogBlock::day_seconds - date;
  }

  inline bool log_hash_exist(ulong hash)
  {
    int i;
    for (i = 0; i <= MIN(this->size, SharedLogBlock::hash_array_size); ++i)
    {
      if (this->log_hash[i] == hash)
      {
        return true;
      }
    }
    return false;
  }

  inline bool append_log_hash(ulong hash)
  {
    if (this->size < SharedLogBlock::hash_array_size - 1)
    {
      this->log_hash[this->size] = hash;
      this->size += 1;
      return true;
    }
    return false;
  }

private:
  static const long hash_array_size = 64;
  static const long day_seconds = 24 * 60 * 60;

  size_t size;
  long date = 0;
  ulong log_hash[SharedLogBlock::hash_array_size];
};

} // namespace openrasp
