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

#include "shared_config_manager.h"
#include "shared_config_block.h"
#include "safe_shutdown_manager.h"

namespace openrasp
{
ShmManager sm;
std::unique_ptr<SharedConfigManager> vcm = nullptr;

SharedConfigManager::SharedConfigManager(ShmManager *mm)
    : BaseManager(mm), variable_config_block(nullptr), rwlock(nullptr)
{
}

int SharedConfigManager::get_check_type_white_bit_mask(std::string url)
{
    DoubleArrayTrie::result_pair_type result_pair[CHECK_TYPE_NR_ITEMS];
    DoubleArrayTrie dat;
    int white_bit_mask = NO_TYPE;
    if (rwlock != nullptr && rwlock->read_try_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        dat.load_existing_array((void *)variable_config_block->get_check_type_white_array(), variable_config_block->get_white_array_size());
        size_t num = dat.prefix_search(url.c_str(), result_pair, sizeof(result_pair));
        if (num > 0)
        {
            for (size_t i = 0; i < num; ++i)
            {
                white_bit_mask |= result_pair[i].value;
            }
        }
    }
    return white_bit_mask;
}

bool SharedConfigManager::write_check_type_white_array_to_shm(const void *source, size_t num)
{
    if (rwlock != nullptr && rwlock->write_try_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        variable_config_block->reset_white_array(source, num);
        return true;
    }
    return false;
}

bool SharedConfigManager::build_check_type_white_array(std::map<std::string, int> &url_mask_map)
{
    std::vector<std::string> urls;
    std::vector<int> values;
    for (auto iter = url_mask_map.begin(); iter != url_mask_map.end(); iter++)
    {
        urls.push_back(iter->first);
        values.push_back(iter->second);
    }
    DoubleArrayTrie::result_pair_type result_pair[CHECK_TYPE_NR_ITEMS];
    DoubleArrayTrie dat;
    dat.build(urls.size(), &urls, 0, &values);
    write_check_type_white_array_to_shm(dat.array(), dat.total_size());
}

bool SharedConfigManager::startup()
{
    char *shm_block = shm_manager->create(SHMEM_SEC_CONF_BLOCK, sizeof(VariableConfigBlock));
    if (!shm_block)
    {
        return false;
    }
    rwlock = new ReadWriteLock((pthread_rwlock_t *)shm_block, LOCK_PROCESS);
    memset(shm_block, 0, sizeof(VariableConfigBlock));
    variable_config_block = reinterpret_cast<VariableConfigBlock *>(shm_block);
    initialized = true;
    return true;
}

bool SharedConfigManager::shutdown()
{
    if (initialized)
    {
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
        if (ssdm != nullptr && !ssdm->is_master_current_process())
        {
            return true;
        }
#endif
        if (rwlock != nullptr)
        {
            delete rwlock;
        }
        shm_manager->destroy(SHMEM_SEC_CONF_BLOCK);
        initialized = false;
    }
    return true;
}

} // namespace openrasp
