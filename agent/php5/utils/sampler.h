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

#include <mutex>

namespace openrasp
{

class Sampler
{
public:
  Sampler(int interval = 0, int burst = 0)
  {
    update(interval, burst);
  }

  void update(int interval, int burst)
  {
    if (this->interval != interval || this->burst != burst)
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (this->interval != interval || this->burst != burst)
      {
        this->interval = interval;
        this->burst = burst;
        this->nextClearTime = 0;
        this->count = 0;
      }
    }
  }

  bool check()
  {
    if (interval <= 0 || burst <= 0)
    {
      return false;
    }
    time_t timestamp = time(nullptr);
    if (timestamp >= nextClearTime)
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (timestamp >= nextClearTime)
      {
        count = 0;
        nextClearTime = timestamp + interval;
        nextClearTime -= nextClearTime % interval;
      }
    }
    if (count >= burst)
    {
      return false;
    }
    std::lock_guard<std::mutex> lock(mtx);
    count++;
    return true;
  }

  std::mutex mtx;
  time_t nextClearTime = 0;
  int count = 0;
  int interval;
  int burst;
};

} // namespace openrasp