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

#include "shared_log_manager.h"

namespace openrasp
{

SharedLogManager::SharedLogManager()
    : shared_log_block(nullptr),
      rwlock(nullptr),
      meta_size(ROUNDUP(sizeof(pthread_rwlock_t), 1 << 3))
{
}

SharedLogManager::~SharedLogManager()
{
  if (rwlock != nullptr)
  {
    delete rwlock;
    rwlock = nullptr;
  }
}

bool SharedLogManager::startup()
{
  size_t total_size = meta_size + sizeof(SharedLogBlock);
  char *shm_block = BaseManager::sm.create(SHMEM_SEC_LOG_BLOCK, total_size);
  if (shm_block)
  {
    memset(shm_block, 0, total_size);
    rwlock = new ReadWriteLock((pthread_rwlock_t *)shm_block, LOCK_PROCESS);
    char *shm_log_block = shm_block + meta_size;
    shared_log_block = reinterpret_cast<SharedLogBlock *>(shm_log_block);
    initialized = true;
    return true;
  }
  return false;
}

bool SharedLogManager::shutdown()
{
  if (initialized)
  {
    if (rwlock != nullptr)
    {
      delete rwlock;
      rwlock = nullptr;
    }
    BaseManager::sm.destroy(SHMEM_SEC_LOG_BLOCK);
    shared_log_block = nullptr;
    initialized = false;
  }
  return true;
}

bool SharedLogManager::log_exist(long timestamp, ulong log_hash)
{
  if (rwlock != nullptr && rwlock->read_try_lock())
  {
    ReadUnLocker auto_unlocker(rwlock);
    if (0 == shared_log_block->compare_date(timestamp) && shared_log_block->log_hash_exist(log_hash))
    {
      return true;
    }
  }
  return false;
}

bool SharedLogManager::log_update(long timestamp, ulong log_hash)
{
  if (rwlock != nullptr && rwlock->write_try_lock())
  {
    WriteUnLocker auto_unlocker(rwlock);
    int date_compare_result = shared_log_block->compare_date(timestamp);
    if (date_compare_result > 0)
    {
      shared_log_block->update_date(timestamp);
    }
    if (shared_log_block->log_hash_exist(log_hash))
    {
      return true;
    }
    else
    {
      shared_log_block->append_log_hash(log_hash);
    }
  }
  return false;
}

} // namespace openrasp
