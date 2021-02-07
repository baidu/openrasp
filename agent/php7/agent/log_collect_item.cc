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

#include "utils/json_reader.h"
#include "utils/file.h"
#include "log_collect_item.h"
#include "openrasp_utils.h"
#include "openrasp_ini.h"
#include "openrasp_log.h"
#include "openrasp_agent.h"
#include "openrasp_agent_manager.h"
#include "shared_config_manager.h"
#include "utils/time.h"

namespace openrasp
{
const long LogCollectItem::time_offset = fetch_time_offset();
const std::string LogCollectItem::status_file = ".status.json";
const std::map<int, const std::string> LogCollectItem::instance_url_map =
    {
        {ALARM_LOGGER, "/v1/agent/log/attack"},
        {POLICY_LOGGER, "/v1/agent/log/policy"},
        {PLUGIN_LOGGER, "/v1/agent/log/plugin"},
        {RASP_LOGGER, "/v1/agent/log/error"}};

const std::map<int, const std::string> LogCollectItem::instance_name_map =
    {
        {ALARM_LOGGER, RaspLoggerEntry::ALARM_LOG_DIR_NAME},
        {POLICY_LOGGER, RaspLoggerEntry::POLICY_LOG_DIR_NAME},
        {PLUGIN_LOGGER, RaspLoggerEntry::PLUGIN_LOG_DIR_NAME},
        {RASP_LOGGER, RaspLoggerEntry::RASP_LOG_DIR_NAME}};

LogCollectItem::LogCollectItem(int instance_id, bool collect_enable)
    : instance_id(instance_id),
      collect_enable(collect_enable)
{
    if (!is_instance_valid())
    {
        error = true;
        return;
    }
    std::string status_file_abs = get_base_dir_path() + LogCollectItem::status_file;
    if (file_exists(status_file_abs))
    {
        std::string status_json;
        if (read_entire_content(status_file_abs, status_json))
        {
            JsonReader json_reader(status_json);
            fpos = json_reader.fetch_int64({"fpos"}, 0);
            st_ino = json_reader.fetch_int64({"st_ino"}, 0);
            last_post_time = json_reader.fetch_int64({"last_post_time"}, 0);
            curr_suffix = json_reader.fetch_string({"curr_suffix"}, curr_suffix);
        }
    }
}

bool LogCollectItem::has_error() const
{
    return error;
}

bool LogCollectItem::get_collect_enable() const
{
    return collect_enable;
}

inline std::string LogCollectItem::get_base_dir_path() const
{
    std::string default_slash(1, DEFAULT_SLASH);
    return std::string(openrasp_ini.root_dir) + default_slash + "logs" + default_slash +
           get_name() + default_slash;
}

inline void LogCollectItem::update_curr_suffix()
{
    curr_suffix = format_time(RaspLoggerEntry::default_log_suffix,
                              strlen(RaspLoggerEntry::default_log_suffix), (long)time(NULL));
}

std::string LogCollectItem::get_active_log_file() const
{
    return get_base_dir_path() + get_name() + ".log." + curr_suffix;
}

void LogCollectItem::update_fpos()
{
    if (!ifs.is_open())
    {
        ifs.open(get_active_log_file(), std::ifstream::binary);
        ifs.seekg(fpos);
    }
    long curr_st_ino = get_active_file_inode();
    if (0 != curr_st_ino && st_ino != curr_st_ino)
    {
        st_ino = curr_st_ino;
        fpos = 0;
    }
}

void LogCollectItem::update_collect_status()
{
    update_fpos();
    if (!ifs.good())
    {
        ifs.clear();
        ifs.sync();
    }
    ifs.sync();
}

long LogCollectItem::get_active_file_inode()
{
    std::string filename = get_active_log_file();
    struct stat sb;
    if (stat(filename.c_str(), &sb) == 0 && (sb.st_mode & S_IFREG) != 0)
    {
        return (long)sb.st_ino;
    }
    return 0;
}

void LogCollectItem::update_status_snapshot()
{
    ifs.clear();
    fpos = ifs.tellg();
    last_post_time = (long)time(NULL);

    JsonReader json_reader;
    json_reader.write_string({"curr_suffix"}, curr_suffix);
    json_reader.write_int64({"last_post_time"}, last_post_time);
    json_reader.write_int64({"fpos"}, fpos);
    json_reader.write_int64({"st_ino"}, st_ino);
    std::string json_content = json_reader.dump(true);

    std::string status_file_abs = get_base_dir_path() + LogCollectItem::status_file;
#ifndef _WIN32
    mode_t oldmask = umask(0);
#endif
    write_string_to_file(status_file_abs.c_str(),
                         std::ofstream::in | std::ofstream::out | std::ofstream::trunc,
                         json_content.c_str(),
                         json_content.length());
#ifndef _WIN32
    umask(oldmask);
#endif
}

std::string LogCollectItem::get_cpmplete_url() const
{
    return std::string(openrasp_ini.backend_url) + get_url_path();
}

bool LogCollectItem::log_content_qualified(const std::string &content)
{
    if (content.empty())
    {
        return false;
    }
    if (nullptr == openrasp_ini.app_id)
    {
        return false;
    }
    JsonReader json_reader(content);
    std::string app_id = json_reader.fetch_string({"app_id"}, "");
    if (app_id != std::string(openrasp_ini.app_id))
    {
        return false;
    }
    std::string rasp_id = json_reader.fetch_string({"rasp_id"}, "");
    if (rasp_id != scm->get_rasp_id())
    {
        return false;
    }
    if (instance_id == RASP_LOGGER)
    {
        static const int collect_level = LEVEL_WARNING;
        std::string level_name = json_reader.fetch_string({"level"}, "");
        int level = RaspLoggerEntry::name_to_level(level_name);
        if (level < 0 || level > collect_level)
        {
            return false;
        }
    }
    return true;
}

void LogCollectItem::refresh_cache_body()
{
    if (0 == cached_count)
    {
        cached_body.push_back('[');
        std::string line;
        while (std::getline(ifs, line) &&
               cached_count < LogAgent::max_post_logs_account)
        {
            if (log_content_qualified(line))
            {
                cached_body.append(line);
                cached_body.push_back(',');
                ++cached_count;
            }
            else
            {
                break;
            }
        }
        cached_body.pop_back();
        cached_body.push_back(']');
        if (0 == cached_count)
        {
            cached_body.clear();
        }
    }
}

void LogCollectItem::clear_cache_body()
{
    cached_count = 0;
    cached_body.clear();
}

std::string LogCollectItem::get_cache_body() const
{
    return cached_body;
}

bool LogCollectItem::need_rotate() const
{
    long now = (long)time(NULL);
    return !same_day_in_current_timezone(now, last_post_time, LogCollectItem::time_offset);
}

void LogCollectItem::handle_rotate(bool need_rotate)
{
    last_post_time = (long)time(NULL);
    if (need_rotate)
    {
        cleanup_expired_logs();
        clear();
    }
}

void LogCollectItem::clear()
{
    update_curr_suffix();
    if (ifs.is_open())
    {
        ifs.close();
        ifs.clear();
    }
    fpos = 0;
    st_ino = 0;
}

void LogCollectItem::cleanup_expired_logs() const
{
    long log_max_backup = 30;
    if (nullptr != scm && scm->get_log_max_backup() > 0)
    {
        log_max_backup = scm->get_log_max_backup();
    }
    std::vector<std::string> files_tobe_deleted;
    long now = (long)time(NULL);
    std::string tobe_deleted_date_suffix =
        format_time(RaspLoggerEntry::default_log_suffix,
                    strlen(RaspLoggerEntry::default_log_suffix), now - log_max_backup * 24 * 60 * 60);
    std::string log_name = get_name();
    openrasp_scandir(get_base_dir_path(), files_tobe_deleted,
                     [&log_name, &tobe_deleted_date_suffix](const char *filename) {
                         return !strncmp(filename, (log_name + ".log.").c_str(), (log_name + ".log.").size()) &&
                                std::string(filename) < (log_name + ".log." + tobe_deleted_date_suffix);
                     },
                     LONG_MAX, true);
    for (std::string delete_file : files_tobe_deleted)
    {
        unlink(delete_file.c_str());
    }
}

const std::string LogCollectItem::get_name() const
{
    auto it = LogCollectItem::instance_name_map.find(instance_id);
    if (it != LogCollectItem::instance_name_map.end())
    {
        return it->second;
    }
    return "unknown";
}
const std::string LogCollectItem::get_url_path() const
{
    auto it = LogCollectItem::instance_url_map.find(instance_id);
    if (it != LogCollectItem::instance_url_map.end())
    {
        return it->second;
    }
    return "unknown";
}

bool LogCollectItem::is_instance_valid() const
{
    auto it = LogCollectItem::instance_name_map.find(instance_id);
    if (it != LogCollectItem::instance_name_map.end())
    {
        return true;
    }
    return false;
}

} // namespace openrasp