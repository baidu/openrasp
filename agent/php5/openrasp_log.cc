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

#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include "openrasp_inject.h"
#include "openrasp_shared_alloc.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

extern "C" {
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/microtime.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
}

ZEND_DECLARE_MODULE_GLOBALS(openrasp_log)

#define RASP_LOG_FILE_MODE                  (mode_t)0666
#define RASP_LOG_TOKEN_REFILL_INTERVAL      60000
#define RASP_STREAM_WRITE_RETRY_NUMBER      1

static bool verify_syslog_address_format(TSRMLS_D);

typedef void (*value_filter_t)(zval *origin_zv, zval *new_zv TSRMLS_DC);

typedef struct keys_filter_t
{
    const std::string origin_key_str;
    const char *new_key_str;
    value_filter_t value_filter;
} keys_filter;

static std::map<std::string, std::string> _if_addr_map;
static char host_name[255];

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
static int openrasp_log_files_mkdir(char *path TSRMLS_DC) { 
    if (VCWD_ACCESS(path, F_OK) == 0)
    {
        return SUCCESS;
    }
    zend_bool mkdir_result = recursive_mkdir(path, strlen(path), 0777 TSRMLS_CC);   
	return mkdir_result ? SUCCESS : FAILURE;
}

static void init_logger_instance(int logger_id TSRMLS_DC)
{
    assert(logger_id >= 0 && logger_id < TOTAL);
    rasp_logger_entry& logger_entry = OPENRASP_LOG_G(loggers)[logger_id];
    if (!logger_entry.initialized)
    {
        logger_entry.available_token = OPENRASP_CONFIG(log.maxburst);
        logger_entry.last_logged_time = get_millisecond(TSRMLS_C);
        char *logger_folder = nullptr;
        spprintf(&logger_folder, 0, "%s%clogs%c%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, logger_entry.name);
        logger_entry.accessable = (openrasp_log_files_mkdir(logger_folder TSRMLS_CC) == SUCCESS) ? 1 : 0;
        efree(logger_folder);
        logger_entry.initialized = 1;
    }
}

static void close_logger_stream(int logger_id TSRMLS_DC)
{
    assert(logger_id >= 0 && logger_id < TOTAL);
    rasp_logger_entry& logger_entry = OPENRASP_LOG_G(loggers)[logger_id];
    if (logger_entry.stream_log)
    {
        php_stream_close(logger_entry.stream_log);
        logger_entry.stream_log = NULL;
    }
}

static void delete_merged_array_keys(HashTable *dest, HashTable *src)
{
    for(zend_hash_internal_pointer_reset(src);
    zend_hash_has_more_elements(src) == SUCCESS;
    zend_hash_move_forward(src)){
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

static zval* _get_ifaddr_zval()
{
	zval* z_ifaddr = NULL;
	MAKE_STD_ZVAL(z_ifaddr);
	array_init(z_ifaddr);
    for (auto iter = _if_addr_map.cbegin(); iter != _if_addr_map.cend(); ++iter) {
        zval* ifa_addr_item = NULL;
        MAKE_STD_ZVAL(ifa_addr_item);
        array_init(ifa_addr_item);
        add_assoc_string(ifa_addr_item, "name", const_cast<char *>(iter->first.c_str()), 1);
        add_assoc_string(ifa_addr_item, "ip", const_cast<char *>(iter->second.c_str()), 1);
        zend_hash_next_index_insert(Z_ARRVAL_P(z_ifaddr), &ifa_addr_item, sizeof(zval *), NULL);
    }
	return z_ifaddr;
}

static std::string fetch_last_clientip(const std::string &s) 
{
    std::stringstream ip_list_ss(s);
    std::string item;
    while (std::getline(ip_list_ss, item, ','))
    ;
    std::stringstream ip_ss(item);
    while (std::getline(ip_ss, item, ' '))
    ;
    return item;
}

static void request_uri_path_filter(zval *origin_zv, zval *new_zv TSRMLS_DC)
{
    char *haystack = Z_STRVAL_P(origin_zv);                                            
    int   haystack_len = Z_STRLEN_P(origin_zv);                
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
    int   haystack_len = Z_STRLEN_P(origin_zv);                
    char* tmp_request_method = estrdup(haystack);
    char *lch = php_strtolower(tmp_request_method, strlen(tmp_request_method));
    ZVAL_STRING(new_zv, lch, 0);
}

static void build_complete_url(zval *items, zval *new_zv TSRMLS_DC)
{
    assert(Z_TYPE_P(items) == IS_ARRAY);
    zval **origin_zv;
    std::string buffer;
    char* request_scheme = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "REQUEST_SCHEME");
    if (request_scheme)
    {
        buffer.append(request_scheme);
    }
    else
    {
        buffer.append(ZEND_STRL("http"));
    }
    buffer.append(ZEND_STRL("://"));
    char* http_host = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "HTTP_HOST");
    if (http_host)
    {
        buffer.append(http_host);
    }
    else
    {
        char* server_name = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_NAME");
        char* server_addr = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_ADDR");
        if (server_name)
        {
            buffer.append(server_name);
        }
        else if (server_addr)
        {
            buffer.append(server_addr);
        }
        char* server_port = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "SERVER_PORT");
        if (server_port && strncmp(server_port, "80", 2) != 0)
        {
            buffer.push_back(':');
            buffer.append(server_port);
        }
    }
    char* request_uri = fetch_outmost_string_from_ht(Z_ARRVAL_P(items), "REQUEST_URI");
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
        char* tmp_clientip_header = estrdup(OPENRASP_CONFIG(clientip.header).c_str());
        char *uch = php_strtoupper(tmp_clientip_header, strlen(tmp_clientip_header));
        const char* server_global_hey = ("HTTP_" + std::string(uch)).c_str();
        total_filters.push_back({server_global_hey, "client_ip", nullptr});
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
                for(std::string item : tokens)
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
    {"REQUEST_METHOD",  "request_method",   request_method_to_lower},
    {"SERVER_NAME",     "target",           nullptr},
    {"SERVER_ADDR",     "server_ip",        nullptr},
    {"HTTP_REFERER",    "referer",          nullptr},
    {"HTTP_USER_AGENT", "user_agent",       nullptr},
    {"REMOTE_ADDR",     "attack_source",    nullptr},
    {"REQUEST_URI",     "path",             request_uri_path_filter},
    {"REQUEST_SCHEME HTTP_HOST SERVER_NAME SERVER_ADDR SERVER_PORT REQUEST_URI",     "url",              build_complete_url}
};

static char* fetch_request_body(size_t max_len TSRMLS_DC)
{
    php_stream *stream = php_stream_open_wrapper("php://input", "rb", 0, NULL);
    if (!stream)
    {
        return estrdup("");
    }
    char *buf = nullptr;
    int len = php_stream_copy_to_mem(stream, &buf, max_len, 0);
    php_stream_close(stream);
    if (len <= 0 || !buf)
    {
        return estrdup("");
    }
    return buf;
}

static void init_alarm_request_info(TSRMLS_D)
{
    assert(OPENRASP_LOG_G(alarm_request_info) == nullptr);
    MAKE_STD_ZVAL(OPENRASP_LOG_G(alarm_request_info));
    array_init(OPENRASP_LOG_G(alarm_request_info));

    zval *migrate_src = nullptr;
    if (PG(http_globals)[TRACK_VARS_SERVER] || zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
    {
        migrate_src = PG(http_globals)[TRACK_VARS_SERVER];
    }
    migrate_hash_values(OPENRASP_LOG_G(alarm_request_info), migrate_src, alarm_filters TSRMLS_CC);    
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "event_type", "attack", 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_hostname", host_name, 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_type", "PHP", 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_version", OPENRASP_PHP_VERSION, 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "request_id", OPENRASP_INJECT_G(request_id), 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "body", fetch_request_body(OPENRASP_CONFIG(body.maxbytes) TSRMLS_CC), 0);
    if (openrasp_ini.app_id)
    {
        add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "app_id", openrasp_ini.app_id, 1);
    }
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp::oam)
    {
        add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "rasp_id", (char *)openrasp::oam->get_rasp_id().c_str(), 1);
        add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "local_ip", (char *)openrasp::oam->get_local_ip(), 1);
    }
    else
    {
        add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "local_ip", "", 1);
    }
#else
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "local_ip", "", 1);
#endif
}

static void init_policy_request_info(TSRMLS_D)
{
    assert(OPENRASP_LOG_G(policy_request_info) == nullptr);
    MAKE_STD_ZVAL(OPENRASP_LOG_G(policy_request_info));
    array_init(OPENRASP_LOG_G(policy_request_info));
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "event_type", "security_policy", 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_hostname", host_name, 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_type", "PHP", 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_version", OPENRASP_PHP_VERSION, 1);
    add_assoc_zval(OPENRASP_LOG_G(policy_request_info), "server_nic", _get_ifaddr_zval());
    if (openrasp_ini.app_id)
    {
        add_assoc_string(OPENRASP_LOG_G(policy_request_info), "app_id", openrasp_ini.app_id, 1);
    }
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp::oam)
    {
        add_assoc_string(OPENRASP_LOG_G(policy_request_info), "rasp_id", (char *)openrasp::oam->get_rasp_id().c_str(), 1);
        add_assoc_string(OPENRASP_LOG_G(policy_request_info), "local_ip", (char *)openrasp::oam->get_local_ip(), 1);
    }
    else
    {
        add_assoc_string(OPENRASP_LOG_G(policy_request_info), "local_ip", "", 1);
    }
#else
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "local_ip", "", 1);
#endif
}

static void clear_alarm_request_info(TSRMLS_D)
{
    assert(Z_TYPE_P(OPENRASP_LOG_G(alarm_request_info)) == IS_ARRAY);
    zval_ptr_dtor(&OPENRASP_LOG_G(alarm_request_info));
    OPENRASP_LOG_G(alarm_request_info) = NULL;
}

static void clear_policy_request_info(TSRMLS_D)
{
    assert(Z_TYPE_P(OPENRASP_LOG_G(policy_request_info)) == IS_ARRAY);
    zval_ptr_dtor(&OPENRASP_LOG_G(policy_request_info));
    OPENRASP_LOG_G(policy_request_info) = NULL;
    
}

/* 初始化logger
*/
static void init_openrasp_loggers(TSRMLS_D)
{    
    for (int logger_id = 0; logger_id < TOTAL; logger_id++)
    {
        init_logger_instance(logger_id TSRMLS_CC);
        if (ALARM_LOGGER == logger_id)
        {
            log_appender alarm_appender = FILE_APPENDER;
            if (verify_syslog_address_format(TSRMLS_C))
            {
                alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
            }
            OPENRASP_LOG_G(loggers)[logger_id].appender = alarm_appender;
        }
    }
}

/* 释放全局logger
*/
static void clear_openrasp_loggers(TSRMLS_D)
{
    for (int logger_id = 0; logger_id < TOTAL; logger_id++)
    {
        close_logger_stream(logger_id TSRMLS_CC);
    }
}

/* 判断是否需要更新文件时间后缀
*/
static zend_bool if_need_update_formatted_file_suffix(rasp_logger_entry *logger, long now, int log_info_len TSRMLS_DC)
{
    int  last_logged_second       = logger->last_logged_time / 1000;
    if (!same_day_in_current_timezone(now, last_logged_second, OPENRASP_LOG_G(time_offset)))
    {
        return 1;
    }
    return 0;
}

static php_stream **openrasp_log_stream_zval_find(rasp_logger_entry *logger, log_appender appender_int TSRMLS_DC)
{
    php_stream *stream              = NULL;
    char       *res                 = NULL;
    long        res_len             = 0l;
    int         need_create_file    = 0;
    char       *file_path           = NULL;
    switch (appender_int)
    {
    case SYSLOG_APPENDER:
        if (!OPENRASP_LOG_G(syslog_stream))
        {
            long now = (long)time(NULL);
            if ((now - OPENRASP_LOG_G(syslog_reconnect_time)) > OPENRASP_CONFIG(syslog_reconnect_interval))
            {
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = OPENRASP_CONFIG(syslog_connection_timeout) * 1000;
                res_len = spprintf(&res, 0, "%s", OPENRASP_CONFIG(syslog_server_address).c_str());
                stream = php_stream_xport_create(res, res_len, REPORT_ERRORS, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, 0, &tv, NULL, NULL, NULL);        
                if (stream)
                {
                    tv.tv_sec = 0;
                    tv.tv_usec = OPENRASP_CONFIG(syslog_read_timeout) * 1000;
                    php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
                    OPENRASP_LOG_G(syslog_stream) = stream;
                    return &OPENRASP_LOG_G(syslog_stream);
                }
                else
                {
                    openrasp_error(E_WARNING, LOG_ERROR, _("Unable to contact syslog server %s"), OPENRASP_CONFIG(syslog_server_address).c_str());                
                }
                efree(res);
                OPENRASP_LOG_G(syslog_reconnect_time) = now;
            }
        }
        else
        {
            return &OPENRASP_LOG_G(syslog_stream);    
        }        
        break;        
    case FILE_APPENDER:
    default:
        if (!logger->stream_log)
        {
            spprintf(&file_path, 0, "%s%clogs%c%s%c%s.log.%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, 
            logger->name, DEFAULT_SLASH, logger->name, OPENRASP_LOG_G(formatted_date_suffix));
            if (VCWD_ACCESS(file_path, F_OK) != 0)
            {
                need_create_file = 1;
            }
            else if (VCWD_ACCESS(file_path, W_OK) != 0)
            {
                openrasp_error(E_WARNING, LOG_ERROR, _("Unable to open '%s' for writing"), file_path);
                efree(file_path);
                break;
            }        
            stream = php_stream_open_wrapper(file_path, "a+", REPORT_ERRORS | IGNORE_URL_WIN, NULL);            
            if (stream)
            {
                if (need_create_file && FAILURE == VCWD_CHMOD(file_path, RASP_LOG_FILE_MODE))
                {
                    openrasp_error(E_WARNING, LOG_ERROR, _("Unable to chmod file: %s."), file_path);
                }                
                logger->stream_log = stream;
                efree(file_path);
                return &logger->stream_log;
            }
            else
            {
                openrasp_error(E_WARNING, LOG_ERROR, _("Fail to open php_stream of %s!"), file_path);
            }
            efree(file_path);
        }
        else
        {
            return &logger->stream_log;
        }        
    }
    return NULL;
}

static int openrasp_log_stream_write(rasp_logger_entry *logger, log_appender appender_int, char *message, int message_len TSRMLS_DC)
{
    php_stream **pp_stream;  
    {
        pp_stream = openrasp_log_stream_zval_find(logger, appender_int TSRMLS_CC);
        if (!pp_stream)
        {
            return FAILURE;
        }
        php_stream_write(*pp_stream, message, message_len);
    }
    return SUCCESS;
}

/* 文件记录日志
*/
static int openrasp_log_by_file(rasp_logger_entry *logger, char *message, int message_len TSRMLS_DC)
{
    long now = (long)time(NULL);    
    if (if_need_update_formatted_file_suffix(logger, now, message_len TSRMLS_CC))
    {
        efree(OPENRASP_LOG_G(formatted_date_suffix));
        OPENRASP_LOG_G(formatted_date_suffix) = openrasp_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now);
        if (logger->stream_log)
        {
            php_stream_close(logger->stream_log);
            logger->stream_log = NULL;
        }
    }

    if (openrasp_log_stream_write(logger, FILE_APPENDER, message, message_len TSRMLS_CC) == FAILURE)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/* TCP UDP记录日志
*/
static int openrasp_log_by_syslog(rasp_logger_entry *logger, severity_level level_int, char *message, int message_len TSRMLS_DC)
{
    char *syslog_info     = NULL;
    char *time_RFC3339    = NULL;
    int   syslog_info_len = 0;
    int   priority        = 0;

    long  now             = (long)time(NULL);
    time_RFC3339 = openrasp_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), now);
    priority = OPENRASP_CONFIG(syslog_facility) * 8 + level_int;
    syslog_info_len = spprintf(&syslog_info, 0, "<%d>%s %s: %s", priority, time_RFC3339, host_name, message);
    efree(time_RFC3339);

    if (openrasp_log_stream_write(logger, SYSLOG_APPENDER, syslog_info, syslog_info_len TSRMLS_CC))
    {
        efree(syslog_info);
        return FAILURE;
    }

    efree(syslog_info);
    return SUCCESS;
}

/*  校验日志level与对应logger的level
 */
static int check_log_level(rasp_logger_entry *logger, severity_level level_int TSRMLS_DC)
{
    if (logger->level >= LEVEL_DEBUG)
    {
        return SUCCESS;
    }
    if (logger->level < LEVEL_INFO)
    {
        return FAILURE;
    }
    switch (level_int)
    {
    case LEVEL_DEBUG:
        if (logger->level >= LEVEL_DEBUG)
        {
            return SUCCESS;
        }
        break;
    case LEVEL_INFO:
        if (logger->level >= LEVEL_INFO)
        {
            return SUCCESS;
        }
        break;
    default:
        return FAILURE;
    }
    return FAILURE;
}

/* 限速实现(bucket token)
*/
static int comsume_token_if_available(rasp_logger_entry *logger TSRMLS_DC)
{
    long now_millisecond = get_millisecond(TSRMLS_C);

    //refill
    if (now_millisecond - logger->last_logged_time > RASP_LOG_TOKEN_REFILL_INTERVAL)
    {
        int refill_amount = (now_millisecond - logger->last_logged_time)
                            / RASP_LOG_TOKEN_REFILL_INTERVAL * OPENRASP_CONFIG(log.maxburst);
        logger->available_token = logger->available_token + refill_amount > OPENRASP_CONFIG(log.maxburst)
                                    ? OPENRASP_CONFIG(log.maxburst)
                                    : logger->available_token + refill_amount;
    }

    //consume
    if(logger->available_token <= 0)
    {
        return FAILURE;
    }
    else
    {
        (logger->available_token)--;
        logger->last_logged_time = get_millisecond(TSRMLS_C);
    }
    return SUCCESS;
}

/* 基础日志方法
*/
static int base_log(rasp_logger_entry *logger, severity_level level_int, char *message, int message_len TSRMLS_DC)
{
    if (!logger->accessable)
    {
        return FAILURE;
    }

    if (check_log_level(logger, level_int TSRMLS_CC) == FAILURE)
    {
        return FAILURE;
    }
    
    if (comsume_token_if_available(logger TSRMLS_CC) == FAILURE)
    {
        return FAILURE;
    }

    if (OPENRASP_LOG_G(in_request_process))
    {
        if (logger->appender & FILE_APPENDER)
        {
            openrasp_log_by_file(logger, message, message_len TSRMLS_CC);
        }
        if (logger->appender & SYSLOG_APPENDER)
        {
            openrasp_log_by_syslog(logger, level_int, message, message_len TSRMLS_CC);
        }
    }
    else
    {
        if (logger->appender & FILE_APPENDER)
        {
            char *file_path = NULL;
            char *tmp_formatted_date_suffix = openrasp_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), (long)time(NULL));
            spprintf(&file_path, 0, "%s%clogs%c%s%c%s.log.%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, 
                logger->name, DEFAULT_SLASH, logger->name, tmp_formatted_date_suffix);
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
            efree(tmp_formatted_date_suffix);
        }
    }

    return SUCCESS;
}

/* 基础info
*/
int base_info(rasp_logger_entry *logger, const char *message, int message_len TSRMLS_DC) {
    return base_log(logger, LEVEL_INFO, (char*)message, message_len TSRMLS_CC);
}

/* 基础debug
*/
static inline int base_debug(rasp_logger_entry *logger, const char *message, int message_len TSRMLS_DC) {
    return base_log(logger, LEVEL_DEBUG, (char*)message, message_len TSRMLS_CC);
}

/*************************************************************************************************/

/* 用于openrasp的info
*/
int rasp_info(const char *message, int message_len TSRMLS_DC) {
    if (!OPENRASP_LOG_G(in_request_process))
    {        
        init_logger_instance(RASP_LOGGER TSRMLS_CC);
    }
    char *rasp_info                 = NULL;    
    char *time_RFC3339 = openrasp_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL));
    int   rasp_info_len = spprintf(&rasp_info, 0, "%s %s\n", time_RFC3339, message);
    int  rasp_result = base_info(&OPENRASP_LOG_G(loggers)[RASP_LOGGER], rasp_info, rasp_info_len TSRMLS_CC);
    efree(time_RFC3339);
    efree(rasp_info);
    return rasp_result;
}

/* 用于插件的info
*/
int plugin_info(const char *message, int message_len TSRMLS_DC) {
    if (!OPENRASP_LOG_G(in_request_process))
    {        
        init_logger_instance(PLUGIN_LOGGER TSRMLS_CC);
    }
    char *plugin_info                 = NULL;    
    char *time_RFC3339 = openrasp_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL));
    int  plugin_info_len = spprintf(&plugin_info, 0, "%s %s", time_RFC3339, message);
    int  plugin_result = base_info(&OPENRASP_LOG_G(loggers)[PLUGIN_LOGGER], plugin_info, plugin_info_len TSRMLS_CC);
    efree(time_RFC3339);
    efree(plugin_info);
    return plugin_result;
}

/* 用于报警的info
*/
int alarm_info(zval *params_result TSRMLS_DC) {
    assert(Z_TYPE_P(params_result) == IS_ARRAY);
    assert(OPENRASP_LOG_G(in_request_process));
    int alarm_result = FAILURE;
    char *event_time = openrasp_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL));
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "event_time", event_time, 1);

    zval *trace = NULL;
	MAKE_STD_ZVAL(trace);
    format_debug_backtrace_str(trace TSRMLS_CC);
    add_assoc_zval(OPENRASP_LOG_G(alarm_request_info), "stack_trace", trace);

    if (php_array_merge(Z_ARRVAL_P(OPENRASP_LOG_G(alarm_request_info)), Z_ARRVAL_P(params_result), 1 TSRMLS_CC))
    {
        smart_str buf_json = {0};
        php_json_encode(&buf_json, OPENRASP_LOG_G(alarm_request_info), 0 TSRMLS_CC);
        if (buf_json.a > buf_json.len)
        {
            buf_json.c[buf_json.len] = '\n';
            buf_json.len++;
        }
        alarm_result = base_info(&OPENRASP_LOG_G(loggers)[ALARM_LOGGER], buf_json.c, buf_json.len TSRMLS_CC);
        smart_str_free(&buf_json);
        
        delete_merged_array_keys(Z_ARRVAL_P(OPENRASP_LOG_G(alarm_request_info)), Z_ARRVAL_P(params_result));
    }
    else
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("Fail to merge request parameters during alarm logging."));
    }
    zend_hash_del(Z_ARRVAL_P(OPENRASP_LOG_G(alarm_request_info)), "stack_trace", sizeof("stack_trace"));                                     
    zend_hash_del(Z_ARRVAL_P(OPENRASP_LOG_G(alarm_request_info)), "event_time", sizeof("event_time"));
    efree(event_time);
    return alarm_result;
}

/* 用于基线的info
*/
int policy_info(zval *params_result TSRMLS_DC) {
    assert(Z_TYPE_P(params_result) == IS_ARRAY);
    if (!OPENRASP_LOG_G(in_request_process))
    {        
        init_logger_instance(POLICY_LOGGER TSRMLS_CC);
        init_policy_request_info(TSRMLS_C);
    }
    
    int policy_result = FAILURE;
    char *event_time = openrasp_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL));
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "event_time", event_time, 1);
    zval *trace = NULL;
    MAKE_STD_ZVAL(trace);
    if (OPENRASP_LOG_G(in_request_process))
    {
        format_debug_backtrace_str(trace TSRMLS_CC);
    }
    else
    {
         ZVAL_STRING(trace, "", 1);
    }
    add_assoc_zval(OPENRASP_LOG_G(policy_request_info), "stack_trace", trace);
    if (php_array_merge(Z_ARRVAL_P(OPENRASP_LOG_G(policy_request_info)), Z_ARRVAL_P(params_result), 1 TSRMLS_CC))
    {
        smart_str buf_json = {0};      
        php_json_encode(&buf_json, OPENRASP_LOG_G(policy_request_info), 0 TSRMLS_CC);
        if (buf_json.a > buf_json.len)
        {
            buf_json.c[buf_json.len] = '\n';
            buf_json.len++;
        }
        policy_result = base_info(&OPENRASP_LOG_G(loggers)[POLICY_LOGGER], buf_json.c, buf_json.len TSRMLS_CC);
        smart_str_free(&buf_json);
        delete_merged_array_keys(Z_ARRVAL_P(OPENRASP_LOG_G(policy_request_info)), Z_ARRVAL_P(params_result));
    }
    else
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("Fail to merge request parameters during policy logging."));
    }
    zend_hash_del(Z_ARRVAL_P(OPENRASP_LOG_G(policy_request_info)), "event_time", sizeof("event_time"));
    efree(event_time);
    if (!OPENRASP_LOG_G(in_request_process))
    {                
        clear_policy_request_info(TSRMLS_C);
    }
    return policy_result;
}

static bool verify_syslog_address_format(TSRMLS_D)
{
    bool result = false;
    bool syslog_alarm_enable = OPENRASP_CONFIG(syslog_alarm_enable);
    std::string syslog_address = OPENRASP_CONFIG(syslog_server_address);
    if (syslog_alarm_enable && !syslog_address.empty()) {
        php_url *resource = php_url_parse_ex(syslog_address.c_str(), syslog_address.length());
        if (resource) {
            if (resource->scheme && (!strcmp(resource->scheme, "tcp") || !strcmp(resource->scheme, "udp"))) {
                result = true;
            }
            else
            {
                openrasp_error(E_WARNING, LOG_ERROR, 
                _("Invalid url scheme in syslog server address: '%s', expecting 'tcp:' or 'udp:'."), syslog_address.c_str());
            }
            php_url_free(resource);
        } else {
            openrasp_error(E_WARNING, LOG_ERROR, 
                _("Invalid syslog server address: '%s', expecting 'tcp://' or 'udp://' to be present."), syslog_address.c_str());
        }
    }
    return result;
}

static void openrasp_log_init_globals(zend_openrasp_log_globals *openrasp_log_globals TSRMLS_DC)
{
	openrasp_log_globals->alarm_request_info 				= NULL;
	openrasp_log_globals->policy_request_info 				= NULL;
	openrasp_log_globals->formatted_date_suffix 			= NULL;	
	openrasp_log_globals->syslog_stream       				= NULL;
    openrasp_log_globals->in_request_process                = 0;
    openrasp_log_globals->syslog_reconnect_time             = 0;

    openrasp_log_globals->loggers[ALARM_LOGGER] = {ALARM_LOG_DIR_NAME,  0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[POLICY_LOGGER] = {POLICY_LOG_DIR_NAME, 0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[PLUGIN_LOGGER] = {PLUGIN_LOG_DIR_NAME, 0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[RASP_LOGGER] = {RASP_LOG_DIR_NAME,   0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};

    log_appender alarm_appender = FILE_APPENDER;
    if (verify_syslog_address_format(TSRMLS_C))
    {
        alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
    }
    openrasp_log_globals->loggers[ALARM_LOGGER].appender     		= alarm_appender;
    openrasp_log_globals->time_offset = fetch_time_offset();
}

PHP_MINIT_FUNCTION(openrasp_log)
{
	ZEND_INIT_MODULE_GLOBALS(openrasp_log, openrasp_log_init_globals, NULL);
    if (check_sapi_need_alloc_shm())
    {
        openrasp_shared_alloc_startup();
    }
    fetch_if_addrs(_if_addr_map);
    if (gethostname(host_name, sizeof(host_name) - 1)) { 
        sprintf( host_name, "UNKNOWN_HOST" );
        openrasp_error(E_WARNING, LOG_ERROR, _("gethostname error: %s"), strerror(errno));
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_log)
{
    if (check_sapi_need_alloc_shm())
    {
        openrasp_shared_alloc_shutdown();
    }
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_log)
{
    OPENRASP_LOG_G(in_request_process) = 1;
	long now = (long)time(NULL);
    OPENRASP_LOG_G(formatted_date_suffix) = openrasp_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now);
    init_openrasp_loggers(TSRMLS_C);
    init_alarm_request_info(TSRMLS_C);               
    init_policy_request_info(TSRMLS_C);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp_log)
{
    if (OPENRASP_LOG_G(formatted_date_suffix))
    {
        efree(OPENRASP_LOG_G(formatted_date_suffix));
        OPENRASP_LOG_G(formatted_date_suffix) = NULL;
    }
    if (OPENRASP_LOG_G(syslog_stream))
    {
        php_stream_close(OPENRASP_LOG_G(syslog_stream));        
        OPENRASP_LOG_G(syslog_stream) = NULL;
    }
    clear_openrasp_loggers(TSRMLS_C);
    clear_alarm_request_info(TSRMLS_C);
    clear_policy_request_info(TSRMLS_C);
    OPENRASP_LOG_G(in_request_process) = 0;
    return SUCCESS;
}
