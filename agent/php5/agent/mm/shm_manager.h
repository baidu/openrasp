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

#ifndef _OPENRASP_MM_SHMEM_MANAGER_H_
#define _OPENRASP_MM_SHMEM_MANAGER_H_

#include <map>

namespace openrasp
{

enum ShmemSecKey
{
  SHMEM_SEC_CTRL_BLOCK,
  SHMEM_SEC_PLUGIN_BLOCK,
  SHMEM_SEC_WEBDIR_BLOCK,
  SHMEM_SEC_CONF_BLOCK,
  SHMEM_SEC_LOG_BLOCK
};

class ShmemSecMeta
{
public:
  char *mem_addr;
  size_t mem_size;
};

class ShmManager
{
public:
  char *create(enum ShmemSecKey mem_key, size_t size);

  int destroy(enum ShmemSecKey mem_key);

private:
  std::map<enum ShmemSecKey, ShmemSecMeta> _shmem_key_map;
};

} // namespace openrasp

#endif
