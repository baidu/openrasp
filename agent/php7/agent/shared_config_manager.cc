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

#include "shared_config_manager.h"
#include "shared_config_block.h"
#include "utils/string.h"
#include "utils/digest.h"
#include "utils/net.h"
#include "utils/hostname.h"
#include "openrasp_v8.h"
#include <algorithm>

namespace openrasp
{
std::unique_ptr<SharedConfigManager> scm = nullptr;

SharedConfigManager::SharedConfigManager()
    : shared_config_block(nullptr), rwlock(nullptr)
{
    meta_size = ROUNDUP(sizeof(pthread_rwlock_t), 1 << 3);
}

SharedConfigManager::~SharedConfigManager()
{
    if (rwlock != nullptr)
    {
        delete rwlock;
        rwlock = nullptr;
    }
}

dat_value SharedConfigManager::get_check_type_white_bit_mask(std::string url)
{
    DoubleArrayTrie dat;
    dat_value white_bit_mask = 0;
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        dat.set_array((void *)shared_config_block->get_check_type_white_array(), shared_config_block->get_white_array_size());
        std::vector<DoubleArrayTrie::result_pair_type> result_pairs = dat.prefix_search(url.c_str());
        for (DoubleArrayTrie::result_pair_type result_pair : result_pairs)
        {
            white_bit_mask |= result_pair.value;
        }
    }
    return white_bit_mask;
}

bool SharedConfigManager::write_check_type_white_array_to_shm(const void *source, size_t num)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->reset_white_array(source, num);
        return true;
    }
    return false;
}

bool SharedConfigManager::build_check_type_white_array(std::map<std::string, dat_value> &url_mask_map)
{
    std::vector<std::string> urls;
    std::vector<dat_value> values;
    for (auto iter = url_mask_map.begin(); iter != url_mask_map.end(); iter++)
    {
        urls.push_back(iter->first);
        values.push_back(iter->second);
    }
    DoubleArrayTrie dat;
    int error = dat.build(urls.size(), &urls, 0, &values);
    if (error < 0 || dat.total_size() > SharedConfigBlock::WHITE_ARRAY_MAX_SIZE)
    {
        return false;
    }
    return write_check_type_white_array_to_shm(dat.array(), dat.total_size());
}

bool SharedConfigManager::build_check_type_white_array(std::map<std::string, std::vector<std::string>> &url_type_map)
{
    std::map<std::string, dat_value> white_mask_map;
    for (auto &white_item : url_type_map)
    {
        dat_value bit_mask = 0;
        if (std::find(white_item.second.begin(), white_item.second.end(), "all") != white_item.second.end())
        {
            bit_mask = (1 << ALL_TYPE) - 1;
        }
        else
        {
            for (auto &type_name : white_item.second)
            {
                bit_mask |= (1 << CheckTypeTransfer::instance().name_to_type(type_name));
            }
        }
        white_mask_map.insert({(white_item.first == "*") ? "" : white_item.first, bit_mask});
    }
    return build_check_type_white_array(white_mask_map);
}

bool SharedConfigManager::build_check_type_white_array(BaseReader *br)
{
    if (!br || br->has_error())
    {
        return false;
    }
    std::vector<std::string> hook_white_key({"hook.white"});
    std::map<std::string, std::vector<std::string>> hook_white_map;
    std::vector<std::string> url_keys = br->fetch_object_keys(hook_white_key);
    for (auto &key_item : url_keys)
    {
        hook_white_key.push_back(key_item);
        std::vector<std::string> white_types = br->fetch_strings(hook_white_key, {});
        hook_white_key.pop_back();
        hook_white_map.insert({key_item, white_types});
    }
    return build_check_type_white_array(hook_white_map);
}

long SharedConfigManager::get_config_last_update()
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->get_config_update_time();
    }
    return 0;
}

bool SharedConfigManager::set_config_last_update(long config_update_timestamp)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->set_config_update_time(config_update_timestamp);
        return true;
    }
    return false;
}

long SharedConfigManager::get_log_max_backup()
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->get_log_max_backup();
    }
    return 0;
}

bool SharedConfigManager::set_log_max_backup(long log_max_backup)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->set_log_max_backup(log_max_backup);
        return true;
    }
    return false;
}

long SharedConfigManager::get_debug_level()
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->get_debug_level();
    }
    return 0;
}

bool SharedConfigManager::set_debug_level(long debug_level)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->set_debug_level(debug_level);
        return true;
    }
    return false;
}

bool SharedConfigManager::set_debug_level(BaseReader *br)
{
    if (!br || br->has_error())
    {
        return false;
    }
    long debug_level = br->fetch_int64({"debug.level"}, 0);
    return set_debug_level(debug_level);
}

bool SharedConfigManager::set_buildin_check_action(std::map<OpenRASPCheckType, OpenRASPActionType> buildin_action_map)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        for (auto &action : buildin_action_map)
        {
            shared_config_block->set_check_type_action(action.first, action.second);
        }
        return true;
    }
    return false;
}

OpenRASPActionType SharedConfigManager::get_buildin_check_action(OpenRASPCheckType check_type)
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->get_check_type_action(check_type);
    }
    return AC_IGNORE;
}

bool SharedConfigManager::startup()
{
    size_t total_size = meta_size + sizeof(SharedConfigBlock);
    char *shm_block = BaseManager::sm.create(SHMEM_SEC_CONF_BLOCK, total_size);
    if (shm_block)
    {
        memset(shm_block, 0, total_size);
        rwlock = new ReadWriteLock((pthread_rwlock_t *)shm_block, LOCK_PROCESS);
        char *shm_config_block = shm_block + meta_size;
        shared_config_block = reinterpret_cast<SharedConfigBlock *>(shm_config_block);
        std::map<std::string, dat_value> all_type_white{{"", ~0}};
        build_check_type_white_array(all_type_white);
        build_rasp_id();
        initialized = true;
        return true;
    }
    return false;
}

bool SharedConfigManager::shutdown()
{
    if (initialized)
    {
        if (rwlock != nullptr)
        {
            delete rwlock;
            rwlock = nullptr;
        }
        BaseManager::sm.destroy(SHMEM_SEC_CONF_BLOCK);
        shared_config_block = nullptr;
        initialized = false;
    }
    return true;
}

bool SharedConfigManager::build_rasp_id()
{
    if (empty(openrasp_ini.rasp_id))
    {
        std::vector<std::string> hw_addrs;
        fetch_hw_addrs(hw_addrs);
        if (hw_addrs.empty())
        {
            return false;
        }
        std::string buf;
        for (auto hw_addr : hw_addrs)
        {
            buf += hw_addr;
        }
        buf.append(get_hostname())
            .append(openrasp_ini.root_dir ? openrasp_ini.root_dir : "")
            .append(sapi_module.name ? sapi_module.name : "");
        this->rasp_id = md5sum(static_cast<const void *>(buf.c_str()), buf.length());
    }
    else
    {
        this->rasp_id = std::string(openrasp_ini.rasp_id);
    }

    return true;
}

std::string SharedConfigManager::get_rasp_id() const
{
    return this->rasp_id;
}

bool SharedConfigManager::write_weak_password_array_to_shm(const void *source, size_t num)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        return shared_config_block->reset_weak_password_array(source, num);
    }
    return false;
}

bool SharedConfigManager::build_weak_password_array(std::vector<std::string> &weak_passwords)
{
    DoubleArrayTrie dat;
    std::sort(weak_passwords.begin(), weak_passwords.end());
    int error = dat.build(weak_passwords.size(), &weak_passwords, 0, 0);
    if (error < 0 || dat.total_size() > SharedConfigBlock::WEAK_PASSWORD_ARRAY_MAX_SIZE)
    {
        return false;
    }
    return write_weak_password_array_to_shm(dat.array(), dat.total_size());
}

bool SharedConfigManager::build_weak_password_array(BaseReader *br)
{
    if (!br || br->has_error())
    {
        return false;
    }
    static std::vector<std::string> dafault_weak_passwords =
        {
            "",
            "root",
            "123",
            "123456",
            "a123456",
            "123456a",
            "111111",
            "123123",
            "admin",
            "user",
            "mysql"};
    std::vector<std::string> hook_white_key({"security.weak_passwords"});
    std::vector<std::string> weak_passwords = br->fetch_strings(hook_white_key, dafault_weak_passwords);
    if (!build_weak_password_array(weak_passwords))
    {
        return build_weak_password_array(dafault_weak_passwords);
    }
    return true;
}

bool SharedConfigManager::is_password_weak(std::string password)
{
    DoubleArrayTrie dat;
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        dat.set_array((void *)shared_config_block->get_weak_password_array(), shared_config_block->get_weak_password_array_size());
        DoubleArrayTrie::result_pair_type result_pair = dat.match_search(password.c_str());
        return result_pair.value != -1;
    }
    return false;
}

void SharedConfigManager::set_mysql_error_codes(std::vector<int64_t> error_codes)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->set_mysql_error_codes(error_codes);
    }
}

bool SharedConfigManager::mysql_error_code_exist(int64_t err_code)
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->mysql_error_code_exist(err_code);
    }
    return false;
}

void SharedConfigManager::set_sqlite_error_codes(std::vector<int64_t> error_codes)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        shared_config_block->set_sqlite_error_codes(error_codes);
    }
}

bool SharedConfigManager::sqlite_error_code_exist(int64_t err_code)
{
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        return shared_config_block->sqlite_error_code_exist(err_code);
    }
    return false;
}

bool SharedConfigManager::write_pg_error_array_to_shm(const void *source, size_t num)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        return shared_config_block->reset_pg_error_array(source, num);
    }
    return false;
}

bool SharedConfigManager::build_pg_error_array(std::vector<std::string> &pg_errors)
{
    DoubleArrayTrie dat;
    std::sort(pg_errors.begin(), pg_errors.end());
    int error = dat.build(pg_errors.size(), &pg_errors, 0, 0);
    if (error < 0 || dat.total_size() > SharedConfigBlock::PG_ERROR_ARRAY_MAX_SIZE)
    {
        return false;
    }
    return write_pg_error_array_to_shm(dat.array(), dat.total_size());
}

bool SharedConfigManager::build_pg_error_array(Isolate *isolate)
{
    if (!isolate)
    {
        return false;
    }
    std::vector<std::string> pg_errs = extract_string_array(isolate, "RASP.algorithmConfig.sql_exception.pgsql.error_code", SharedConfigBlock::PGSQL_ERROR_CODE_MAX_SIZE);
    return build_pg_error_array(pg_errs);
}

bool SharedConfigManager::pg_error_filtered(std::string error)
{
    DoubleArrayTrie dat;
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        dat.set_array((void *)shared_config_block->get_pg_error_array(), shared_config_block->get_pg_error_array_size());
        DoubleArrayTrie::result_pair_type result_pair = dat.match_search(error.c_str());
        return result_pair.value != -1;
    }
    return false;
}

bool SharedConfigManager::write_env_key_array_to_shm(const void *source, size_t num)
{
    if (rwlock != nullptr && rwlock->write_lock())
    {
        WriteUnLocker auto_unlocker(rwlock);
        return shared_config_block->reset_env_key_array(source, num);
    }
    return false;
}

bool SharedConfigManager::build_env_key_array(std::vector<std::string> &env_keys)
{
    DoubleArrayTrie dat;
    std::sort(env_keys.begin(), env_keys.end());
    int error = dat.build(env_keys.size(), &env_keys, 0, 0);
    if (error < 0 || dat.total_size() > SharedConfigBlock::ENV_KEY_ARRAY_MAX_SIZE)
    {
        return false;
    }
    return write_env_key_array_to_shm(dat.array(), dat.total_size());
}

bool SharedConfigManager::build_env_key_array(Isolate *isolate)
{
    if (!isolate)
    {
        return false;
    }
    std::vector<std::string> pg_errs = extract_string_array(isolate, "RASP.algorithmConfig.webshell_ld_preload.env", SharedConfigBlock::WEBSHELL_ENV_KEY_MAX_SIZE);
    return build_env_key_array(pg_errs);
}

bool SharedConfigManager::filter_env_key(const std::string &env)
{
    DoubleArrayTrie dat;
    bool found = false;
    if (rwlock != nullptr && rwlock->read_lock())
    {
        ReadUnLocker auto_unlocker(rwlock);
        dat.set_array((void *)shared_config_block->get_env_key_array(), shared_config_block->get_env_key_array_size());
        std::vector<DoubleArrayTrie::result_pair_type> result_pairs = dat.prefix_search(env.c_str());
        for (DoubleArrayTrie::result_pair_type result_pair : result_pairs)
        {
            if (result_pair.value != -1 &&
                result_pair.length <= env.length() - 1 &&
                '=' == env[result_pair.length])
            {
                found = true;
            }
        }
    }
    return found;
}

} // namespace openrasp
