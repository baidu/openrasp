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

#ifndef _READ_WRITE_LOCK_H_
#define _READ_WRITE_LOCK_H_

#include <pthread.h>

namespace openrasp
{

enum LOCK_TYPE
{
  LOCK_PROCESS = 0,
  LOCK_THREAD
};

class ReadWriteLock
{
public:
  ReadWriteLock(pthread_rwlock_t *rwlock, enum LOCK_TYPE lock_type);
  ~ReadWriteLock();

  bool read_unlock();
  bool read_try_lock();
  bool read_lock();

  bool write_unlock();
  bool write_try_lock();
  bool write_lock();

private:
  int lock_type;
  pthread_rwlock_t *rwlock;
  pthread_rwlockattr_t rwlock_attr;
};

class ReadUnLocker
{
public:
  ReadUnLocker(ReadWriteLock *lock) : rlock(lock) {}
  ~ReadUnLocker() { rlock->read_unlock(); }

private:
  ReadWriteLock *rlock;
};

class WriteUnLocker
{
public:
  WriteUnLocker(ReadWriteLock *lock) : wlock(lock) {}
  ~WriteUnLocker() { wlock->write_unlock(); }

private:
  ReadWriteLock *wlock;
};

} // namespace openrasp

#endif