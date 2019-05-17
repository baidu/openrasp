/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
}

using openrasp::fetch_if_addrs;
using openrasp::fetch_time_offset;
using openrasp::format_time;
using openrasp::regex_match;
using openrasp::same_day_in_current_timezone;

std::unique_ptr<openrasp::SharedLogManager> slm = nullptr;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_log)

#define RASP_LOG_FILE_MODE (mode_t)0666
#define RASP_LOG_TOKEN_REFILL_INTERVAL 60000
#define RASP_STREAM_WRITE_RETRY_NUMBER 1

static bool verify_syslog_address_format(TSRMLS_D);

typedef void (*value_filter_t)(zval *origin_zv, zval *new_zv TSRMLS_DC);

typedef struct keys_filter_t
{
    const std::string origin_key_str;
    const char *new_key_str;
    value_filter_t value_filter;
} keys_filter;

static bool is_initialized = false;
static std::map<std::string, std::string> _if_addr_map;

/* 获取当前毫秒时间
*/
static long get_millisecond(TSRMLS_D)
{
    struct timeval tp = {0};
    gettimeofday(&tp, NULL);
    return (long)(tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

/* 创建日志文件夹
*/
static int openrasp_log_files_mkdir(char *path TSRMLS_DC)
{
    if (VCWD_ACCESS(path, F_OK) == 0)
    {
        return SUCCESS;
    }
    zend_bool mkdir_result = recursive_mkdir(path, strlen(path), 0777 TSRMLS_CC);
    return mkdir_result ? SUCCESS : FAILURE;
}

static void delete_merged_array_keys(HashTable *dest, HashTable *src)
{
    for (zend_hash_internal_pointer_reset(src);
         zend_hash_has_more_elements(src) == SUCCESS;
         zend_hash_move_forward(src))
    {
        char *key;
        uint keylen;
        ulong idx;
        int type = zend_hash_get_current_key_ex(src, &key, &keylen, &idx, 0, NULL);
        if (type == HASH_KEY_IS_STRING)
        {
            zend_hash_del(dest, key, keylen);
        }
    }
}

static zval *_get_ifaddr_zval()
{
    zval *z_ifaddr = NULL;
    MAKE_STD_ZVAL(z_ifaddr);
    array_init(z_ifaddr);
    for (auto iter = _if_addr_map.cbegin(); iter != _if_addr_map.cend(); ++iter)
    {
        zval *ifa_addr_item = NULL;
        MAKE_STD_ZVAL(ifa_addr_item);
        array_init(ifa_addr_item);
        add_assoc_string(ifa_addr_item, "name", const_cast<char *>(iter->first.c_str()), 1);
        add_assoc_string(ifa_addr_item, "ip", const_cast<char *>(iter->second.c_str()), 1);
        zend_hash_next_index_insert(Z_ARRVAL_P(z_ifaddr), &ifa_addr_item, sizeof(zval *), NULL);
    }
    return z_ifaddr;
}

static void request_uri_path_filter(zval *origin_zv, zval *new_zv TSRMLS_DC)
{
    char *haystack = Z_STRVAL_P(origin_zv);
    int haystack_len = Z_STRLEN_P(origin_zv);
    const char *found = php_memnstr(haystack, "?", 1, haystack + haystack_len);
    if (found)
    {
        ZVAL_STRINGL(new_zv, haystack, found - haystack, 1);
    }
    else
    {
        ZVAL_STRING(new_zv, haystack, 1);
    }
}

static void request_method_to_lower(zval *origin_zv, zval *new_zv TSRMLS_DC)
{
    char *haystack = Z_STRVAL_P(origin_zv);
    int haystack_len = Z_STRLEN_P(origin_zv);
    char *tmp_request_method = estrdup(haystack);
    char *lch = php_strtolower(tmp_request_method, strlen(tmp_request_method));
    ZVAL_STRING(new_zv, lch, 0);
}

static void build_complete_url(zval *items, zval *new_zv TSRMLS_DC)
{
    assert(Z_TYPE_P(items) == IS_ARRAY);
    zval **origin_zv;
    std::string buffer;
    char *request_scheme = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "REQUEST_SCHEME");
    if (request_scheme)
    {
        buffer.append(request_scheme);
    }
    else
    {
        buffer.append(ZEND_STRL("http"));
    }
    buffer.append(ZEND_STRL("://"));
    char *http_host = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "HTTP_HOST");
    if (http_host)
    {
        buffer.append(http_host);
    }
    else
    {
        char *server_name = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_NAME");
        char *server_addr = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_ADDR");
        if (server_name)
        {
            buffer.append(server_name);
        }
        else if (server_addr)
        {
            buffer.append(server_addr);
        }
        char *server_port = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_PORT");
        if (server_port && strcmp(server_port, "80") != 0)
        {
            buffer.push_back(':');
            buffer.append(server_port);
        }
    }
    char *request_uri = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "REQUEST_URI");
    if (request_uri)
    {
        buffer.append(request_uri);
    }
    ZVAL_STRINGL(new_zv, buffer.c_str(), buffer.length(), 1);
}

static void migrate_hash_values(zval *dest, const zval *src, std::vector<keys_filter> &filters TSRMLS_DC)
{
    std::vector<keys_filter> total_filters(filters);
    if (!OPENRASP_CONFIG(clientip.header).empty())
    {
        char *tmp_clientip_header = estrdup(OPENRASP_CONFIG(clientip.header).c_str());
        char *uch = php_strtoupper(tmp_clientip_header, strlen(tmp_clientip_header));
        total_filters.push_back({("HTTP_" + std::string(uch)), "client_ip", nullptr});
        efree(tmp_clientip_header);
    }
    zval **origin_zv;
    for (keys_filter filter : total_filters)
    {
        if (src && Z_TYPE_P(src) == IS_ARRAY)
        {
            if (filter.origin_key_str.find(" ") != std::string::npos)
            {
                zval *items = nullptr;
                MAKE_STD_ZVAL(items);
                array_init(items);
                std::istringstream iss(filter.origin_key_str);
                std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
                for (std::string item : tokens)
                {
                    if (zend_hash_find(Z_ARRVAL_P(src), item.c_str(), item.size() + 1, (void **)&origin_zv) == SUCCESS &&
                        Z_TYPE_PP(origin_zv) == IS_STRING)
                    {
                        add_assoc_zval(items, item.c_str(), *origin_zv);
                        Z_ADDREF_PP(origin_zv);
                    }
                }
                assert(filter.value_filter != nullptr);
                if (filter.value_filter)
                {
                    zval *new_zv = nullptr;
                    MAKE_STD_ZVAL(new_zv);
                    filter.value_filter(items, new_zv TSRMLS_CC);
                    add_assoc_zval(dest, filter.new_key_str, new_zv);
                    zval_ptr_dtor(&items);
                }
            }
            else
            {
                if (zend_hash_find(Z_ARRVAL_P(src), filter.origin_key_str.c_str(), filter.origin_key_str.size() + 1, (void **)&origin_zv) == SUCCESS &&
                    Z_TYPE_PP(origin_zv) == IS_STRING)
                {
                    if (filter.value_filter)
                    {
                        zval *new_zv = nullptr;
                        MAKE_STD_ZVAL(new_zv);
                        filter.value_filter(*origin_zv, new_zv TSRMLS_CC);
                        add_assoc_zval(dest, filter.new_key_str, new_zv);
                    }
                    else
                    {
                        add_assoc_zval(dest, filter.new_key_str, *origin_zv);
                        Z_ADDREF_PP(origin_zv);
                    }
                }
            }
        }
        if (zend_hash_find(Z_ARRVAL_P(dest), filter.new_key_str, strlen(filter.new_key_str) + 1, (void **)&origin_zv) == FAILURE)
        {
            add_assoc_string(dest, filter.new_key_str, "", 1);
        }
    }
}

static std::vector<keys_filter> alarm_filters =
    {
        {"REQUEST_METHOD", "request_method", request_method_to_lower},
        {"SERVER_NAME", "target", nullptr},
        {"SERVER_ADDR", "server_ip", nullptr},
        {"REMOTE_ADDR", "attack_source", nullptr},
        {"REQUEST_URI", "path", request_uri_path_filter},
        {"REQUEST_SCHEME HTTP_HOST SERVER_NAME SERVER_ADDR SERVER_PORT REQUEST_URI", "url", build_complete_url}};

static bool verify_syslog_address_format(TSRMLS_D)
{
    bool result = false;
    bool syslog_alarm_enable = OPENRASP_CONFIG(syslog.enable);
    std::string syslog_address = OPENRASP_CONFIG(syslog.url);
    if (syslog_alarm_enable && !syslog_address.empty())
    {
        php_url *resource = php_url_parse_ex(syslog_address.c_str(), syslog_address.length());
        if (resource)
        {
            if (resource->scheme && (!strcmp(resource->scheme, "tcp") || !strcmp(resource->scheme, "udp")))
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

static void openrasp_log_init_globals(zend_openrasp_log_globals *openrasp_log_globals TSRMLS_DC)
{
    openrasp_log_globals->in_request_process = 0;
    log_appender alarm_appender = FSTREAM_APPENDER;
    if (verify_syslog_address_format(TSRMLS_C))
    {
        alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
    }
    openrasp_log_globals->alarm_logger = std::move(RaspLoggerEntry(ALARM_LOG_DIR_NAME, LEVEL_INFO, alarm_appender, static_cast<log_appender>(FSTREAM_APPENDER | SYSLOG_APPENDER)));
    openrasp_log_globals->policy_logger = std::move(RaspLoggerEntry(POLICY_LOG_DIR_NAME, LEVEL_INFO, FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));
    openrasp_log_globals->plugin_logger = std::move(RaspLoggerEntry(PLUGIN_LOG_DIR_NAME, LEVEL_INFO, FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));
    openrasp_log_globals->rasp_logger = std::move(RaspLoggerEntry(RASP_LOG_DIR_NAME, LEVEL_INFO, FSTREAM_APPENDER, static_cast<log_appender>(FSTREAM_APPENDER | FILE_APPENDER)));
}

static void openrasp_log_shutdown_globals(zend_openrasp_log_globals *openrasp_log_globals TSRMLS_DC)
{
    //make sure release dynamic alloc memory
    openrasp_log_globals->alarm_logger.clear(TSRMLS_C);
    openrasp_log_globals->policy_logger.clear(TSRMLS_C);
    openrasp_log_globals->plugin_logger.clear(TSRMLS_C);
    openrasp_log_globals->rasp_logger.clear(TSRMLS_C);
}

PHP_MINIT_FUNCTION(openrasp_log)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_log, openrasp_log_init_globals, openrasp_log_shutdown_globals);
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
    OPENRASP_LOG_G(alarm_logger).init(static_cast<log_appender>(FSTREAM_APPENDER | SYSLOG_APPENDER) TSRMLS_CC);
    OPENRASP_LOG_G(plugin_logger).init(FSTREAM_APPENDER TSRMLS_CC);
    OPENRASP_LOG_G(policy_logger).init(FSTREAM_APPENDER TSRMLS_CC);
    OPENRASP_LOG_G(rasp_logger).init(FSTREAM_APPENDER TSRMLS_CC);
    OPENRASP_LOG_G(rasp_logger).set_level(openrasp::scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp_log)
{
    OPENRASP_LOG_G(alarm_logger).clear(TSRMLS_C);
    OPENRASP_LOG_G(plugin_logger).clear(TSRMLS_C);
    OPENRASP_LOG_G(policy_logger).clear(TSRMLS_C);
    OPENRASP_LOG_G(rasp_logger).clear(TSRMLS_C);
    OPENRASP_LOG_G(in_request_process) = 0;
    return SUCCESS;
}

const char *RaspLoggerEntry::default_log_suffix = "%Y-%m-%d";
const char *RaspLoggerEntry::rasp_rfc3339_format = "%Y-%m-%dT%H:%M:%S%z";
const char *RaspLoggerEntry::syslog_time_format = "%b %d %H:%M:%S";

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

void RaspLoggerEntry::init(log_appender appender_int TSRMLS_DC)
{
    if (!initialized)
    {
        available_token = OPENRASP_CONFIG(log.maxburst);
        last_logged_time = get_millisecond(TSRMLS_C);
        char *logger_folder = nullptr;
        spprintf(&logger_folder, 0, "%s%clogs%c%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, name);
        accessable = (openrasp_log_files_mkdir(logger_folder TSRMLS_CC) == SUCCESS) ? 1 : 0;
        efree(logger_folder);
        time_offset = fetch_time_offset();
        initialized = 1;
    }
    appender = static_cast<log_appender>(appender_int & appender_mask);
    if (appender & SYSLOG_APPENDER)
    {
        bool syslog_available = verify_syslog_address_format(TSRMLS_C);
        if (!syslog_available)
        {
            appender = static_cast<log_appender>(appender & ~SYSLOG_APPENDER);
        }
    }
    update_formatted_date_suffix();
    update_common_info(TSRMLS_C);
}

void RaspLoggerEntry::clear(TSRMLS_D)
{
    close_streams(TSRMLS_C);
    clear_common_info();
    clear_formatted_date_suffix();
}

void RaspLoggerEntry::update_formatted_date_suffix()
{
    if (FSTREAM_APPENDER & appender)
    {
        long now = (long)time(NULL);
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

void RaspLoggerEntry::close_streams(TSRMLS_D)
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

bool RaspLoggerEntry::comsume_token_if_available(TSRMLS_D)
{
    long now_millisecond = get_millisecond(TSRMLS_C);

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
        last_logged_time = get_millisecond(TSRMLS_C);
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

bool RaspLoggerEntry::openrasp_log_stream_available(log_appender appender_int TSRMLS_DC)
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
                                                 STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, 0, &tv, NULL, NULL, NULL);
                if (stream)
                {
                    tv.tv_sec = 0;
                    tv.tv_usec = OPENRASP_CONFIG(syslog.read_timeout) * 1000;
                    php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
                    syslog_stream = stream;
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

bool RaspLoggerEntry::raw_log(severity_level level_int, const char *message, int message_len TSRMLS_DC)
{
    if (!accessable)
    {
        return false;
    }
    if (!check_log_level(level_int))
    {
        return false;
    }
    if (!comsume_token_if_available(TSRMLS_C))
    {
        return false;
    }

    if (appender & FSTREAM_APPENDER)
    {
        long now = (long)time(NULL);
        if (if_need_update_formatted_file_suffix(now))
        {
            update_formatted_date_suffix();
            close_streams(TSRMLS_C);
        }
        if (openrasp_log_stream_available(FSTREAM_APPENDER TSRMLS_CC))
        {
            php_stream_write(stream_log, message, message_len);
        }
    }
    if (appender & SYSLOG_APPENDER)
    {
        if (openrasp_log_stream_available(SYSLOG_APPENDER TSRMLS_CC))
        {
            char *syslog_info = NULL;
            int syslog_info_len = 0;

            long now = (long)time(NULL);
            std::string syslog_time = format_time(RaspLoggerEntry::syslog_time_format, strlen(RaspLoggerEntry::syslog_time_format), now);
            int priority = OPENRASP_CONFIG(syslog.facility) * 8 + level_int;
            std::string tag = OPENRASP_CONFIG(syslog.tag);
            syslog_info_len = spprintf(&syslog_info, 0, "<%d>%s %s %s[%d]: %s",
                                       priority,
                                       syslog_time.c_str(),
                                       openrasp::scm->get_hostname().c_str(),
                                       tag.c_str(),
                                       getpid(),
                                       message);
            php_stream_write(syslog_stream, syslog_info, syslog_info_len);
            efree(syslog_info);
        }
    }

    if (appender & FILE_APPENDER)
    {
        char *file_path = NULL;
        std::string tmp_formatted_date_suffix = format_time(RaspLoggerEntry::default_log_suffix,
                                                            strlen(RaspLoggerEntry::default_log_suffix), (long)time(NULL));
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

bool RaspLoggerEntry::log(severity_level level_int, const char *message, int message_len TSRMLS_DC, bool separate, bool detail)
{
    bool in_request = OPENRASP_LOG_G(in_request_process);
    if (!in_request) //out of request
    {
        init(FILE_APPENDER TSRMLS_CC);
    }
    bool log_result = false;
    std::string complete_log;
    if (detail)
    {
        std::string time_RFC3339 = format_time(RaspLoggerEntry::rasp_rfc3339_format,
                                               strlen(RaspLoggerEntry::rasp_rfc3339_format), (long)time(NULL));
        complete_log.append(time_RFC3339 + " ");
    }
    complete_log.append(message);
    if (separate)
    {
        complete_log.push_back('\n');
    }
    log_result = raw_log(level_int, complete_log.c_str(), complete_log.length() TSRMLS_CC);
    if (!in_request) //out of request
    {
        clear(TSRMLS_C);
    }
    return log_result;
}

bool RaspLoggerEntry::log(severity_level level_int, zval *z_message TSRMLS_DC)
{
    assert(Z_TYPE_P(z_message) == IS_ARRAY);
    bool in_request = OPENRASP_LOG_G(in_request_process);
    if (!in_request) //out of request
    {
        init(FILE_APPENDER TSRMLS_CC);
    }
    bool log_result = false;
    {
        std::string event_time = format_time(RaspLoggerEntry::rasp_rfc3339_format,
                                             strlen(RaspLoggerEntry::rasp_rfc3339_format), (long)time(NULL));
        add_assoc_string(common_info, "event_time", estrdup(event_time.c_str()), 0);
    }
    {
        zval *trace = NULL;
        MAKE_STD_ZVAL(trace);
        if (in_request)
        {
            format_debug_backtrace_str(trace TSRMLS_CC);
        }
        else
        {
            ZVAL_STRING(trace, "", 1);
        }
        add_assoc_zval(common_info, "stack_trace", trace);
    }
    zval *source_code_arr = NULL;
    MAKE_STD_ZVAL(source_code_arr);
    array_init(source_code_arr);
    if (OPENRASP_CONFIG(decompile.enable) && in_request)
    {
        format_source_code_arr(source_code_arr TSRMLS_CC);
    }
    add_assoc_zval(common_info, "source_code", source_code_arr);

    if (php_array_merge(Z_ARRVAL_P(common_info), Z_ARRVAL_P(z_message), 1 TSRMLS_CC))
    {
        std::string str_message = json_encode_from_zval(common_info TSRMLS_CC);
        str_message.push_back('\n');
        log_result = raw_log(level_int, str_message.c_str(), str_message.length() TSRMLS_CC);
        delete_merged_array_keys(Z_ARRVAL_P(common_info), Z_ARRVAL_P(z_message));
    }
    else
    {
        RaspLoggerEntry::inner_error(E_WARNING, LOG_ERROR, _("Fail to merge request parameters during %s logging."), name);
    }
    if (OPENRASP_CONFIG(decompile.enable))
    {
        zend_hash_del(Z_ARRVAL_P(common_info), "source_code", sizeof("source_code"));
    }
    zend_hash_del(Z_ARRVAL_P(common_info), "stack_trace", sizeof("stack_trace"));
    zend_hash_del(Z_ARRVAL_P(common_info), "event_time", sizeof("event_time"));
    if (!in_request) //out of request
    {
        clear(TSRMLS_C);
    }
    return log_result;
}

void RaspLoggerEntry::update_common_info(TSRMLS_D)
{
    if (common_info)
    {
        return;
    }
    MAKE_STD_ZVAL(common_info);
    array_init(common_info);
    if (strcmp(name, ALARM_LOG_DIR_NAME) == 0 &&
        (appender & appender_mask))
    {
        zval *migrate_src = nullptr;
        if (PG(http_globals)[TRACK_VARS_SERVER] || zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
        {
            migrate_src = PG(http_globals)[TRACK_VARS_SERVER];
        }
        migrate_hash_values(common_info, migrate_src, alarm_filters TSRMLS_CC);
        add_assoc_string(common_info, "event_type", "attack", 1);
        add_assoc_string(common_info, "server_hostname", (char *)openrasp::scm->get_hostname().c_str(), 1);
        add_assoc_string(common_info, "server_type", "php", 1);
        add_assoc_string(common_info, "server_version", OPENRASP_PHP_VERSION, 1);
        add_assoc_string(common_info, "request_id", OPENRASP_INJECT_G(request_id), 1);
        add_assoc_string(common_info, "body", fetch_request_body(OPENRASP_CONFIG(body.maxbytes) TSRMLS_CC), 0);
        add_assoc_zval(common_info, "server_nic", _get_ifaddr_zval());
        add_assoc_string(common_info, "rasp_id", (char *)openrasp::scm->get_rasp_id().c_str(), 1);
        if (openrasp_ini.app_id)
        {
            add_assoc_string(common_info, "app_id", openrasp_ini.app_id, 1);
        }

        zval *z_header = NULL;
        MAKE_STD_ZVAL(z_header);
        array_init(z_header);
        if (migrate_src)
        {
            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(migrate_src));
                 zend_hash_has_more_elements(Z_ARRVAL_P(migrate_src)) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(migrate_src)))
            {
                char *key;
                ulong idx;
                int type;
                zval **value;
                std::string header_key;
                type = zend_hash_get_current_key(Z_ARRVAL_P(migrate_src), &key, &idx, 0);
                if (type == HASH_KEY_IS_STRING &&
                    !(header_key = convert_to_header_key(key, strlen(key))).empty() &&
                    zend_hash_get_current_data(Z_ARRVAL_P(migrate_src), (void **)&value) == SUCCESS &&
                    Z_TYPE_PP(value) == IS_STRING)
                {
                    add_assoc_zval(z_header, header_key.c_str(), *value);
                    Z_ADDREF_PP(value);
                }
            }
        }
        add_assoc_zval(common_info, "header", z_header);
    }
    else if (strcmp(name, POLICY_LOG_DIR_NAME) == 0 &&
             (appender & appender_mask))
    {
        add_assoc_string(common_info, "event_type", "security_policy", 1);
        add_assoc_string(common_info, "server_hostname", (char *)openrasp::scm->get_hostname().c_str(), 1);
        add_assoc_string(common_info, "server_type", "php", 1);
        add_assoc_string(common_info, "server_version", OPENRASP_PHP_VERSION, 1);
        add_assoc_zval(common_info, "server_nic", _get_ifaddr_zval());
        add_assoc_string(common_info, "rasp_id", (char *)openrasp::scm->get_rasp_id().c_str(), 1);
        if (openrasp_ini.app_id)
        {
            add_assoc_string(common_info, "app_id", openrasp_ini.app_id, 1);
        }
    }
}

void RaspLoggerEntry::clear_common_info()
{
    if (common_info)
    {
        zval_ptr_dtor(&common_info);
        common_info = nullptr;
    }
}

char *RaspLoggerEntry::get_formatted_date_suffix() const
{
    return formatted_date_suffix;
}

zval *RaspLoggerEntry::get_common_info(TSRMLS_D) const
{
    return common_info;
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
    TSRMLS_FETCH();
    LOG_G(rasp_logger).set_level(openrasp::scm->get_debug_level() != 0 ? LEVEL_DEBUG : LEVEL_INFO);
}

std::map<std::string, std::string> get_if_addr_map()
{
    return _if_addr_map;
}