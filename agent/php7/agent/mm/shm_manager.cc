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

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "openrasp.h"
#include "shm_manager.h"

namespace openrasp
{

char *ShmManager::create(enum ShmemSecKey mem_key, size_t mem_size)
{
    std::map<enum ShmemSecKey, ShmemSecMeta>::iterator it = _shmem_key_map.find(mem_key);
    if (it != _shmem_key_map.end())
    {
        if (it->second.mem_size != mem_size)
        {
            return NULL;
        }
        return it->second.mem_addr;
    }

    char *mem_addr = NULL;
    mem_addr = reinterpret_cast<char *>(
        mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0));

    ShmemSecMeta shmem_meta;
    shmem_meta.mem_size = mem_size;
    shmem_meta.mem_addr = mem_addr;
    _shmem_key_map[mem_key] = shmem_meta;

    return mem_addr;
}

int ShmManager::destroy(enum ShmemSecKey mem_key)
{
    std::map<enum ShmemSecKey, ShmemSecMeta>::iterator it = _shmem_key_map.find(mem_key);
    if (it == _shmem_key_map.end())
    {
        return FAILURE;
    }
    ShmemSecMeta &shmem_meta = it->second;

    munmap(shmem_meta.mem_addr, shmem_meta.mem_size);

    _shmem_key_map.erase(mem_key);

    return SUCCESS;
}

} // namespace openrasp
