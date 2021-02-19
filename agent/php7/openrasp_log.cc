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

#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include "openrasp_inject.h"
#include "utils/regex.h"
#include "utils/time.h"
#include "utils/net.h"
#include "utils/hostname.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "agent/shared_config_manager.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

extern "C"
{
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/microtime.h"
#include "ext/date/php_date.h"
#include "zend_smart_str.h"
#include "ext/json/php_json.h"
}

using openrasp::fetch_if_addrs;
using openrasp::fetch_time_offset;
using openrasp::format_time;
using openrasp::regex_match;
using openrasp::same_day_in_current_timezone;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_log)

#define RASP_LOG_FILE_MODE (mode_t)0666

static const int RASP_LOG_TOKEN_REFILL_INTERVAL = 60000;
std::unique_ptr<openrasp::SharedLogManager> slm = nullptr;

static bool verify_syslog_address_format();

static bool is_initialized = false;
static std::map<std::string, std::string> _if_addr_map;

/* 获取当前毫秒时间
*/
static long get_millisecond()
{
    struct timeval tp = {0};
    gettimeofday(&tp, nullptr);
    return (long)(tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

/* 创建日志文件夹
*/
static int openrasp_log_files_mkdir(char *path)
{
    if (VCWD_ACCESS(path, F_OK) == 0)
    {
        return SUCCESS;
    }
    zend_bool mkdir_result = recursive_mkdir(path, strlen(path), 0777);
    return mkdir_result ? SUCCESS : FAILURE;
}

static bool verify_syslog_address_format()
{
    bool result = false;
    bool syslog_alarm_enable = OPENRASP_CONFIG(syslog.enable);
    std::string syslog_address = OPENRASP_CONFIG(syslog.url);
    if (syslog_alarm_enable && !syslog_address.empty())
    {
        php_url *resource = php_url_parse_ex(syslog_address.c_str(), syslog_address.length());
        if (resource)
        {
            std::string scheme;
            if (resource->scheme)
            {
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3)
                scheme = std::string(resource->scheme);
#else
                scheme = std::string(resource->scheme->val, resource->scheme->len);
#endif
            }
            if (scheme.find("tcp") == 0 || scheme.find("udp") == 0)
            {
                result = true;
            }
            else
            {
                RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR,
                                             _("Invalid url scheme in syslog server address: '%s', expecting 'tcp:' or 'udp:'."), syslog_address.c_str());
            }
            php_url_free(resource);
        }
        else
        {
            RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR,
                                         _("Invalid syslog server address: '%s', expecting 'tcp://' or 'udp://' to be present."), syslog_address.c_str());
        }
    }
    return result;
}

static void openrasp_log_init_globals(zend_openrasp_log_globals *openrasp_log_globals)
{
    openrasp_log_globals->in_request_process = 0;
    log_appender alarm_appender = FSTREAM_APPENDER;
    if (verify_syslog_address_format())
    {
        alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
    }
    openrasp_log_globals->alarm_logger =
        std::move(RaspLoggerEntry(RaspLoggerEntry::ALARM_LOG_DIR_NAME, LEVEL_INFO,
                                  alarm_appender, static_cast<log_appender>(FSTREAM_APPENDER | SYSLOG_APPENDER)));

    openrasp_log_globals->policy_logger =
        std::move(RaspLoggerEntry(RaspLoggerEntry::POLICY_LOG_DIR_NAME, LEVEL_INFO,
                                  FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));

    openrasp_log_globals->plugin_logger =
        std::move(RaspLoggerEntry(RaspLoggerEntry::PLUGIN_LOG_DIR_NAME, LEVEL_INFO,
                                  FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));

    openrasp_log_globals->rasp_logger =
        std::move(RaspLoggerEntry(RaspLoggerEntry::RASP_LOG_DIR_NAME, LEVEL_INFO,
                                  FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));
}

PHP_MINIT_FUNCTION(openrasp_log)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_log, openrasp_log_init_globals, nullptr);
    if (need_alloc_shm_current_sapi())
    {
        slm.reset(new openrasp::SharedLogManager());
        slm->startup();
    }
    fetch_if_addrs(_if_addr_map);
    is_initialized = true;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_log)
{
    if (need_alloc_shm_current_sapi() && slm != nullptr)
    {
        slm->shutdown();
    }
    is_initialized = false;
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_log)
{
    OPENRASP_LOG_G(in_request_process) = 1;
    OPENRASP_LOG_G(alarm_logger).init(static_cast<log_appender>(FSTREAM_APPENDER | SYSLOG_APPENDER));
    OPENRASP_LOG_G(plugin_logger).init(FSTREAM_APPENDER);
    OPENRASP_LOG_G(policy_logger).init(FSTREAM_APPENDER);
    OPENRASP_LOG_G(rasp_logger).init(FSTREAM_APPENDER);
    OPENRASP_LOG_G(rasp_logger).set_level(openrasp::scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp_log)
{
    OPENRASP_LOG_G(alarm_logger).clear();
    OPENRASP_LOG_G(plugin_logger).clear();
    OPENRASP_LOG_G(policy_logger).clear();
    OPENRASP_LOG_G(rasp_logger).clear();
    OPENRASP_LOG_G(in_request_process) = 0;
    return SUCCESS;
}

const char *RaspLoggerEntry::default_log_suffix = "%Y-%m-%d";
const char *RaspLoggerEntry::rasp_rfc3339_format = "%Y-%m-%dT%H:%M:%S%z";
const char *RaspLoggerEntry::syslog_time_format = "%b %d %H:%M:%S";
const char *RaspLoggerEntry::ALARM_LOG_DIR_NAME = "alarm";
const char *RaspLoggerEntry::POLICY_LOG_DIR_NAME = "policy";
const char *RaspLoggerEntry::PLUGIN_LOG_DIR_NAME = "plugin";
const char *RaspLoggerEntry::RASP_LOG_DIR_NAME = "rasp";

RaspLoggerEntry::RaspLoggerEntry()
    : name("invalid")
{
}

RaspLoggerEntry::RaspLoggerEntry(const char *name, severity_level level, log_appender appender, log_appender appender_mask)
    : name(name),
      level(level),
      appender(appender),
      appender_mask(appender_mask)
{
    syslog_reconnect_time = 0;
}

void RaspLoggerEntry::init(log_appender appender_int)
{
    if (!initialized)
    {
        available_token = OPENRASP_CONFIG(log.maxburst);
        last_logged_time = get_millisecond();
        char *logger_folder = nullptr;
        spprintf(&logger_folder, 0, "%s%clogs%c%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, name);
        accessable = (openrasp_log_files_mkdir(logger_folder) == SUCCESS) ? 1 : 0;
        efree(logger_folder);
        time_offset = fetch_time_offset();
        initialized = 1;
    }
    appender = static_cast<log_appender>(appender_int & appender_mask);
    if (appender & SYSLOG_APPENDER)
    {
        bool syslog_available = verify_syslog_address_format();
        if (!syslog_available)
        {
            appender = static_cast<log_appender>(appender & ~SYSLOG_APPENDER);
        }
    }
    update_formatted_date_suffix();
}

void RaspLoggerEntry::clear()
{
    close_streams();
    clear_formatted_date_suffix();
}

void RaspLoggerEntry::update_formatted_date_suffix()
{
    if (FSTREAM_APPENDER & appender)
    {
        long now = (long)time(nullptr);
        if (formatted_date_suffix != nullptr)
        {
            efree(formatted_date_suffix);
        }
        formatted_date_suffix = estrdup(
            format_time(
                RaspLoggerEntry::default_log_suffix,
                strlen(RaspLoggerEntry::default_log_suffix), now)
                .c_str());
    }
}

void RaspLoggerEntry::clear_formatted_date_suffix()
{
    if (formatted_date_suffix != nullptr)
    {
        efree(formatted_date_suffix);
        formatted_date_suffix = nullptr;
    }
}

void RaspLoggerEntry::close_streams()
{
    if (nullptr != stream_log)
    {
        php_stream_close(stream_log);
        stream_log = nullptr;
    }
    if (nullptr != syslog_stream)
    {
        php_stream_close(syslog_stream);
        syslog_stream = nullptr;
    }
}

bool RaspLoggerEntry::check_log_level(severity_level level_int) const
{
    if (level < LEVEL_EMERG)
    {
        return false;
    }
    return (level >= level_int);
}

bool RaspLoggerEntry::comsume_token_if_available()
{
    long now_millisecond = get_millisecond();

    //refill
    if (now_millisecond - last_logged_time > RASP_LOG_TOKEN_REFILL_INTERVAL)
    {
        int refill_amount = (now_millisecond - last_logged_time) /
                            RASP_LOG_TOKEN_REFILL_INTERVAL * OPENRASP_CONFIG(log.maxburst);
        available_token = available_token + refill_amount > OPENRASP_CONFIG(log.maxburst)
                              ? OPENRASP_CONFIG(log.maxburst)
                              : available_token + refill_amount;
    }

    //consume
    if (available_token <= 0)
    {
        return false;
    }
    else
    {
        (available_token)--;
        last_logged_time = get_millisecond();
    }
    return true;
}

bool RaspLoggerEntry::if_need_update_formatted_file_suffix(long now) const
{
    int last_logged_second = last_logged_time / 1000;
    if (!same_day_in_current_timezone(now, last_logged_second, time_offset))
    {
        return true;
    }
    return false;
}

bool RaspLoggerEntry::openrasp_log_stream_available(log_appender appender_int)
{
    php_stream *stream = nullptr;
    char *res = nullptr;
    long res_len = 0l;
    int need_create_file = 0;
    char *file_path = nullptr;
    switch (appender_int)
    {
    case SYSLOG_APPENDER:
        if (nullptr != syslog_stream)
        {
            return true;
        }
        else
        {
            long now = (long)time(nullptr);
            if ((now - syslog_reconnect_time) > OPENRASP_CONFIG(syslog.reconnect_interval))
            {
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = OPENRASP_CONFIG(syslog.connection_timeout) * 1000;
                res_len = spprintf(&res, 0, "%s", OPENRASP_CONFIG(syslog.url).c_str());
                stream = php_stream_xport_create(res, res_len, REPORT_ERRORS,
                                                 STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, 0, &tv, nullptr, nullptr, nullptr);
                if (stream)
                {
                    tv.tv_sec = 0;
                    tv.tv_usec = OPENRASP_CONFIG(syslog.read_timeout) * 1000;
                    php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
                    syslog_stream = stream;
                    efree(res);
                    return true;
                }
                else
                {
                    RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR,
                                                 _("Unable to contact syslog server %s"), OPENRASP_CONFIG(syslog.url).c_str());
                }
                efree(res);
                syslog_reconnect_time = now;
            }
        }
        break;
    case FSTREAM_APPENDER:
    default:
        if (nullptr != stream_log)
        {
            return true;
        }
        else
        {
            spprintf(&file_path, 0, "%s%clogs%c%s%c%s.log.%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH,
                     name, DEFAULT_SLASH, name, formatted_date_suffix);
            if (VCWD_ACCESS(file_path, F_OK) != 0)
            {
                need_create_file = 1;
            }
            else if (VCWD_ACCESS(file_path, W_OK) != 0)
            {
                RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR, _("Unable to open '%s' for writing"), file_path);
                efree(file_path);
                break;
            }
            stream = php_stream_open_wrapper(file_path, "a+", REPORT_ERRORS | IGNORE_URL_WIN, nullptr);
            if (stream)
            {
                if (need_create_file && FAILURE == VCWD_CHMOD(file_path, RASP_LOG_FILE_MODE))
                {
                    RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR, _("Unable to chmod file: %s."), file_path);
                }
                stream_log = stream;
                efree(file_path);
                return true;
            }
            else
            {
                RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR, _("Fail to open php_stream of %s!"), file_path);
            }
            efree(file_path);
        }
    }
    return false;
}

bool RaspLoggerEntry::raw_log(severity_level level_int, const char *message, int message_len)
{
    if (!accessable)
    {
        return false;
    }
    if (!check_log_level(level_int))
    {
        return false;
    }
    if (!comsume_token_if_available())
    {
        return false;
    }

    if (appender & FSTREAM_APPENDER)
    {
        long now = (long)time(nullptr);
        if (if_need_update_formatted_file_suffix(now))
        {
            update_formatted_date_suffix();
            close_streams();
        }
        if (openrasp_log_stream_available(FSTREAM_APPENDER))
        {
            php_stream_write(stream_log, message, message_len);
        }
    }
    if (appender & SYSLOG_APPENDER)
    {
        if (openrasp_log_stream_available(SYSLOG_APPENDER))
        {
            char *syslog_info = nullptr;
            int syslog_info_len = 0;

            long now = (long)time(nullptr);
            std::string syslog_time = format_time(RaspLoggerEntry::syslog_time_format, strlen(RaspLoggerEntry::syslog_time_format), now);
            int priority = OPENRASP_CONFIG(syslog.facility) * 8 + level_int;
            std::string tag = OPENRASP_CONFIG(syslog.tag);
            syslog_info_len = spprintf(&syslog_info, 0, "<%d>%s %s %s[%d]: %s",
                                       priority,
                                       syslog_time.c_str(),
                                       openrasp::get_hostname().c_str(),
                                       tag.c_str(),
                                       getpid(),
                                       message);
            php_stream_write(syslog_stream, syslog_info, syslog_info_len);
            efree(syslog_info);
        }
    }

    if (appender & FILE_APPENDER)
    {
        char *file_path = nullptr;
        std::string tmp_formatted_date_suffix = format_time(RaspLoggerEntry::default_log_suffix,
                                                            strlen(RaspLoggerEntry::default_log_suffix), (long)time(nullptr));
        spprintf(&file_path, 0, "%s%clogs%c%s%c%s.log.%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH,
                 name, DEFAULT_SLASH, name, tmp_formatted_date_suffix.c_str());
#ifndef _WIN32
        mode_t oldmask = umask(0);
#endif
        FILE *fp = fopen(file_path, "a+");
        if (fp)
        {
            fwrite(message, sizeof(char), message_len, fp);
            fclose(fp);
        }
#ifndef _WIN32
        umask(oldmask);
#endif
        efree(file_path);
    }

    return true;
}

bool RaspLoggerEntry::log(severity_level level_int, const char *message, int message_len, bool separate, bool detail)
{
    bool in_request = OPENRASP_LOG_G(in_request_process);
    if (!in_request) //out of request
    {
        init(FILE_APPENDER);
    }
    bool log_result = false;
    std::string complete_log;
    if (detail)
    {
        std::string time_RFC3339 = format_time(RaspLoggerEntry::rasp_rfc3339_format,
                                               strlen(RaspLoggerEntry::rasp_rfc3339_format), (long)time(nullptr));
        complete_log.append(time_RFC3339 + " ");
        if (in_request)
        {
            complete_log.append(OPENRASP_G(request).url.get_request_scheme())
                .append("://")
                .append(OPENRASP_G(request).url.get_real_host())
                .append(OPENRASP_G(request).url.get_path())
                .append(" ");
        }
    }
    complete_log.append(message);
    if (separate)
    {
        complete_log.push_back('\n');
    }
    log_result = raw_log(level_int, complete_log.c_str(), complete_log.length());
    if (!in_request) //out of request
    {
        clear();
    }
    return log_result;
}

bool RaspLoggerEntry::log(severity_level level_int, openrasp::JsonReader &base_json)
{
    openrasp::JsonReader j;
    bool in_request = OPENRASP_LOG_G(in_request_process);
    if (!in_request) //out of request
    {
        init(FILE_APPENDER);
    }
    bool log_result = false;
    if (openrasp_ini.app_id)
    {
        j.write_string({"app_id"}, openrasp_ini.app_id);
    }
    j.write_string({"server_hostname"}, openrasp::get_hostname());
    j.write_string({"server_type"}, "php");
    j.write_string({"server_version"}, get_phpversion());
    j.write_string({"rasp_id"}, openrasp::scm->get_rasp_id());
    j.write_map_to_array({"server_nic"}, "name", "ip", _if_addr_map);
    std::string event_time = format_time(RaspLoggerEntry::rasp_rfc3339_format,
                                         strlen(RaspLoggerEntry::rasp_rfc3339_format), (long)time(NULL));
    j.write_string({"event_time"}, event_time);
    std::vector<std::string> source_code_vec;
    if (OPENRASP_CONFIG(decompile.enable))
    {
        source_code_vec = format_source_code_arr();
    }
    j.write_vector({"source_code"}, source_code_vec);
    if (strcmp(name, RaspLoggerEntry::ALARM_LOG_DIR_NAME) == 0 &&
        (appender & appender_mask))
    {
        j.write_string({"event_type"}, "attack");
        j.write_string({"request_id"}, OPENRASP_G(request).get_id());
        j.write_string({"request_method"}, OPENRASP_G(request).get_method());
        j.write_string({"target"}, OPENRASP_G(request).url.get_server_name());
        j.write_string({"server_ip"}, OPENRASP_G(request).url.get_server_addr());
        j.write_string({"path"}, OPENRASP_G(request).url.get_path());
        j.write_string({"url"}, OPENRASP_G(request).url.get_complete_url());
        j.write_string({"attack_source"}, OPENRASP_G(request).get_remote_addr());
        j.write_map({"header"}, OPENRASP_G(request).get_header());
        std::string clientip_header = OPENRASP_CONFIG(clientip.header);
        std::transform(clientip_header.begin(), clientip_header.end(), clientip_header.begin(), ::tolower);
        j.write_string({"client_ip"}, OPENRASP_G(request).get_header(clientip_header));
        j.write_string({"body"}, OPENRASP_G(request).get_parameter().get_body());
        j.write_string({"parameter", "form"}, OPENRASP_G(request).get_parameter().get_form_str());
        j.write_string({"parameter", "json"}, OPENRASP_G(request).get_parameter().get_json_str());
        j.write_string({"parameter", "multipart"}, OPENRASP_G(request).get_parameter().get_multipart_str());
    }
    else if (strcmp(name, RaspLoggerEntry::POLICY_LOG_DIR_NAME) == 0 &&
             (appender & appender_mask))
    {
        j.write_string({"event_type"}, "security_policy");
    }
    j.update(base_json);
    std::string str_message = j.dump();
    str_message.push_back('\n');
    log_result = raw_log(level_int, str_message.c_str(), str_message.length());
    if (!in_request) //out of request
    {
        clear();
    }
    return log_result;
}

char *RaspLoggerEntry::get_formatted_date_suffix() const
{
    return formatted_date_suffix;
}

void RaspLoggerEntry::set_level(severity_level level)
{
    this->level = level;
}

std::string RaspLoggerEntry::level_to_name(severity_level level)
{
    static const std::map<severity_level, const std::string> level_name_map =
        {{LEVEL_EMERG, "EMERG"},
         {LEVEL_ALERT, "ALERT"},
         {LEVEL_CRIT, "CRIT"},
         {LEVEL_ERR, "ERROR"},
         {LEVEL_WARNING, "WARN"},
         {LEVEL_NOTICE, "NOTICE"},
         {LEVEL_DEBUG, "DEBUG"}};
    auto it = level_name_map.find(level);
    if (it != level_name_map.end())
    {
        return it->second;
    }
    else
    {
        return "UNKNOWN";
    }
}

int RaspLoggerEntry::name_to_level(const std::string &name)
{
    static const std::map<const std::string, int> name_level_map =
        {{"EMERG", LEVEL_EMERG},
         {"ALERT", LEVEL_ALERT},
         {"CRIT", LEVEL_CRIT},
         {"ERROR", LEVEL_ERR},
         {"WARN", LEVEL_WARNING},
         {"NOTICE", LEVEL_NOTICE},
         {"DEBUG", LEVEL_DEBUG}};
    auto it = name_level_map.find(name);
    if (it != name_level_map.end())
    {
        return it->second;
    }
    else
    {
        return -1;
    }
}

void RaspLoggerEntry::inner_error(int type, openrasp_error_code code, const char *format, ...)
{
    va_list arg;
    char *message = nullptr;
    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    zend_error(type, "[OpenRASP] %d %s", code, message);
    efree(message);
}

bool log_module_initialized()
{
    return is_initialized;
}

void update_log_level()
{
    LOG_G(rasp_logger).set_level(openrasp::scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
}

std::map<std::string, std::string> get_if_addr_map()
{
    return _if_addr_map;
}
