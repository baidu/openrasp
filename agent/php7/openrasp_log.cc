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
#include "openrasp_inject.h"
#include <map>
#include <vector>

extern "C" {
#include "openrasp_shared_alloc.h"
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/microtime.h"
#include "ext/date/php_date.h"
#include "zend_smart_str.h"
#include "ext/json/php_json.h"

#ifdef PHP_WIN32
# include "win32/time.h"
# include <windows.h>
# if defined(HAVE_IPHLPAPI_WS2)
#  include <winsock2.h>
#  include <iphlpapi.h>
#  define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#  define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
# endif
#elif defined(NETWARE)
# include <sys/timeval.h>
# include <sys/time.h>
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
# include <sys/types.h>
# include <ifaddrs.h>
# include <netinet/in.h>
# include <netdb.h>
# include <sys/socket.h>
# include <net/if.h>
#else
# include <unistd.h>
# include <sys/time.h>
#endif
}

ZEND_DECLARE_MODULE_GLOBALS(openrasp_log)

#define RASP_LOG_FILE_MODE                  (mode_t)0666
#define DEFAULT_LOG_FILE_SUFFIX             "Y-m-d"
#define RASP_RFC3339_FORMAT                 "Y-m-d\\TH:i:sP"
#define RASP_LOG_TOKEN_REFILL_INTERVAL      1000
#define RASP_STREAM_WRITE_RETRY_NUMBER      1

typedef void (*value_filter_t)(zval *origin_zv, zval *new_zv);

typedef struct keys_filter_t
{
    const char *origin_key_str;
    const char *new_key_str;
    value_filter_t value_filter;
} keys_filter;

static std::map<std::string, std::string> _if_addr_map;
static char host_name[255];

/* 获取当前毫秒时间
*/
static long get_millisecond()
{
    struct timeval tp = {0};
    gettimeofday(&tp, NULL);
    return (long)(tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

/* 创建日志文件夹
*/
static int openrasp_log_files_mkdir(char *path) { 
    if (VCWD_ACCESS(path, F_OK) == 0)
    {
        return SUCCESS;
    }
    zend_bool mkdir_result = recursive_mkdir(path, strlen(path), 0777);   
	return mkdir_result ? SUCCESS : FAILURE;
}

static void init_logger_instance(int logger_id)
{
    assert(logger_id >= 0 && logger_id < TOTAL);
    rasp_logger_entry& logger_entry = OPENRASP_LOG_G(loggers)[logger_id];
    if (!logger_entry.initialized)
    {
        logger_entry.available_token = openrasp_ini.log_maxburst;
        logger_entry.last_logged_time = get_millisecond();
        char *logger_folder = NULL;
        spprintf(&logger_folder, 0, "%s%clogs%c%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, logger_entry.name);
        logger_entry.accessable = (openrasp_log_files_mkdir(logger_folder) == SUCCESS) ? 1 : 0;
        efree(logger_folder);
        logger_entry.initialized = 1;
    }
}

static void close_logger_stream(int logger_id)
{
    assert(logger_id >= 0 && logger_id < TOTAL);
    rasp_logger_entry& logger_entry = OPENRASP_LOG_G(loggers)[logger_id];
    if (logger_entry.stream_log)
    {
        php_stream_close(logger_entry.stream_log);
        logger_entry.stream_log = NULL;
    }
}

static void delete_merged_array_keys(HashTable *dest, const HashTable *src)
{
    zend_string *skey;
    ZEND_HASH_FOREACH_STR_KEY(dest, skey) 
    {
        if (zend_hash_exists(src, skey)) 
        {
            zend_hash_del(dest, skey);
        }
    } ZEND_HASH_FOREACH_END();
}

static void _get_ifaddr_zval(zval *z_ifaddr)
{
	array_init(z_ifaddr);
    for (auto iter = _if_addr_map.cbegin(); iter != _if_addr_map.cend(); ++iter) {
        zval ifa_addr_item;
        array_init(&ifa_addr_item);
        add_assoc_str(&ifa_addr_item, "name", zend_string_init(iter->first.c_str(), iter->first.length(), 0));
        add_assoc_str(&ifa_addr_item, "ip", zend_string_init(iter->second.c_str(), iter->second.length(), 0));
        zend_hash_next_index_insert(Z_ARRVAL_P(z_ifaddr), &ifa_addr_item);
    }
}

static void request_uri_path_filter(zval *origin_zv, zval *new_zv)
{
    char *haystack = Z_STRVAL_P(origin_zv);                                            
    int   haystack_len = Z_STRLEN_P(origin_zv);                
    const char *found = php_memnstr(haystack, "?", 1, haystack + haystack_len);
    if (found)
    {
        ZVAL_STRINGL(new_zv, haystack, found - haystack);
    }
}

static void migrate_hash_values(zval *dest, const zval *src, std::vector<keys_filter> &filters)
{
    zval *origin_zv;
    for (keys_filter filter:filters)
    {
        if (src && Z_TYPE_P(src) == IS_ARRAY &&
        (origin_zv = zend_hash_str_find(Z_ARRVAL_P(src), filter.origin_key_str, strlen(filter.origin_key_str))) != nullptr &&
        Z_TYPE_P(origin_zv) == IS_STRING)
        {
            if (filter.value_filter)
            {
                zval new_zv;
                filter.value_filter(origin_zv, &new_zv);
                add_assoc_zval(dest, filter.new_key_str, &new_zv);
            }
            else
            {
                add_assoc_zval(dest, filter.new_key_str, origin_zv);
                Z_TRY_ADDREF_P(origin_zv);
            }
        }
        else
        {
            add_assoc_string(dest, filter.new_key_str, "");
        }
    }
}

static void init_alarm_request_info()
{
    static std::vector<keys_filter> alarm_filters = 
    {
        {"REMOTE_ADDR",     "attack_source",    nullptr},
        {"SERVER_NAME",     "target",           nullptr},
        {"SERVER_ADDR",     "server_ip",        nullptr},
        {"REQUEST_URI",     "url",              nullptr},
        {"HTTP_REFERER",    "referer",          nullptr},
        {"HTTP_USER_AGENT", "user_agent",       nullptr},
        {"REQUEST_URI",     "path",             request_uri_path_filter}
    };

    assert(Z_TYPE(OPENRASP_LOG_G(alarm_request_info)) == IS_NULL);
    array_init(&OPENRASP_LOG_G(alarm_request_info));
    zval *migrate_src = nullptr;
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY || zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        migrate_src = &PG(http_globals)[TRACK_VARS_SERVER];
    }
    migrate_hash_values(&OPENRASP_LOG_G(alarm_request_info), migrate_src, alarm_filters);
    add_assoc_string(&OPENRASP_LOG_G(alarm_request_info), "event_type", "attack");
    add_assoc_string(&OPENRASP_LOG_G(alarm_request_info), "server_hostname", host_name);
    add_assoc_string(&OPENRASP_LOG_G(alarm_request_info), "server_type", "PHP");
    add_assoc_string(&OPENRASP_LOG_G(alarm_request_info), "server_version", OPENRASP_PHP_VERSION);
    add_assoc_string(&OPENRASP_LOG_G(alarm_request_info), "request_id", OPENRASP_INJECT_G(request_id));
}

static void init_policy_request_info()
{
    assert(Z_TYPE(OPENRASP_LOG_G(policy_request_info)) == IS_NULL);
    array_init(&OPENRASP_LOG_G(policy_request_info));
    add_assoc_string(&OPENRASP_LOG_G(policy_request_info), "event_type", "security_policy");
    add_assoc_string(&OPENRASP_LOG_G(policy_request_info), "server_hostname", host_name);
    add_assoc_string(&OPENRASP_LOG_G(policy_request_info), "server_type", "PHP");
    add_assoc_string(&OPENRASP_LOG_G(policy_request_info), "server_version", OPENRASP_PHP_VERSION);
    zval ifaddr;
    _get_ifaddr_zval(&ifaddr);
    add_assoc_zval(&OPENRASP_LOG_G(policy_request_info), "server_nic", &ifaddr);
}

static void clear_alarm_request_info()
{
    assert(Z_TYPE(OPENRASP_LOG_G(alarm_request_info)) == IS_ARRAY);
    zval_ptr_dtor(&OPENRASP_LOG_G(alarm_request_info));
    ZVAL_NULL(&OPENRASP_LOG_G(alarm_request_info));
}

static void clear_policy_request_info()
{
    assert(Z_TYPE(OPENRASP_LOG_G(policy_request_info)) == IS_ARRAY);
    zval_ptr_dtor(&OPENRASP_LOG_G(policy_request_info));
    ZVAL_NULL(&OPENRASP_LOG_G(policy_request_info));
}

/* 初始化logger
*/
static void init_openrasp_loggers()
{
    for (int logger_id = 0; logger_id < TOTAL; logger_id++)
    {
        init_logger_instance(logger_id);
    }
}

/* 释放全局logger
*/
static void clear_openrasp_loggers()
{
    for (int logger_id = 0; logger_id < TOTAL; logger_id++)
    {
        close_logger_stream(logger_id);
    }
}

/* 判断是否需要更新文件时间后缀
*/
static zend_bool if_need_update_formatted_file_suffix(rasp_logger_entry *logger, long now, int log_info_len)
{
    int  last_logged_second       = logger->last_logged_time / 1000;
    long log_rotate_second        = 24*60*60;
    if (now/log_rotate_second != last_logged_second/log_rotate_second)
    {
        return 1;
    }    
    return 0;
}

static php_stream **openrasp_log_stream_zval_find(rasp_logger_entry *logger, log_appender appender_int)
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
            if ((now - OPENRASP_LOG_G(last_retry_time)) > openrasp_ini.syslog_connection_retry_interval)
            {
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = openrasp_ini.syslog_connection_timeout * 1000;
                res_len = spprintf(&res, 0, "%s", openrasp_ini.syslog_server_address);
                stream = php_stream_xport_create(res, res_len, REPORT_ERRORS, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, 0, &tv, NULL, NULL, NULL);        
                if (stream)
                {
                    tv.tv_sec = 0;
                    tv.tv_usec = openrasp_ini.syslog_read_timeout * 1000;
                    php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
                    OPENRASP_LOG_G(syslog_stream) = stream;
                    return &OPENRASP_LOG_G(syslog_stream);
                }
                else
                {
                    openrasp_error(E_WARNING, LOG_ERROR, _("Fail to connect syslog server %s."), openrasp_ini.syslog_server_address);                
                }
                efree(res);
                OPENRASP_LOG_G(last_retry_time) = now;
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
            logger->name, DEFAULT_SLASH, logger->name, ZSTR_VAL(OPENRASP_LOG_G(formatted_date_suffix)));
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

static int openrasp_log_stream_write(rasp_logger_entry *logger, log_appender appender_int, char *message, int message_len)
{
    php_stream **pp_stream;  
    {
        pp_stream = openrasp_log_stream_zval_find(logger, appender_int);
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
static int openrasp_log_by_file(rasp_logger_entry *logger, char *message, int message_len)
{
    long now = (long)time(NULL);    
    if (if_need_update_formatted_file_suffix(logger, now, message_len))
    {
        zend_string_release(OPENRASP_LOG_G(formatted_date_suffix));
        OPENRASP_LOG_G(formatted_date_suffix) = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now, 1);
        if (logger->stream_log)
        {
            php_stream_close(logger->stream_log);
            logger->stream_log = NULL;
        }
    }

    if (openrasp_log_stream_write(logger, FILE_APPENDER, message, message_len) == FAILURE)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/* TCP UDP记录日志
*/
static int openrasp_log_by_syslog(rasp_logger_entry *logger, severity_level level_int, char *message, int message_len)
{
    char *syslog_info     = NULL;
    zend_string *time_RFC3339    = NULL;
    int   syslog_info_len = 0;
    int   priority        = 0;

    long  now             = (long)time(NULL);
    time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), now, 1);
    priority = openrasp_ini.syslog_facility * 8 + level_int;
    syslog_info_len = spprintf(&syslog_info, 0, "<%d>%s %s: %s", priority, ZSTR_VAL(time_RFC3339), host_name, message);
    zend_string_release(time_RFC3339);

    if (openrasp_log_stream_write(logger, SYSLOG_APPENDER, syslog_info, syslog_info_len))
    {
        efree(syslog_info);
        return FAILURE;
    }

    efree(syslog_info);
    return SUCCESS;
}

/*  校验日志level与对应logger的level
 */
static int check_log_level(rasp_logger_entry *logger, severity_level level_int)
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
static int comsume_token_if_available(rasp_logger_entry *logger)
{
    long now_millisecond = get_millisecond();

    //refill
    if (now_millisecond - logger->last_logged_time > RASP_LOG_TOKEN_REFILL_INTERVAL)
    {
        int refill_amount = (now_millisecond - logger->last_logged_time)
                            / RASP_LOG_TOKEN_REFILL_INTERVAL * openrasp_ini.log_maxburst;
        logger->available_token = logger->available_token + refill_amount > openrasp_ini.log_maxburst
                                    ? openrasp_ini.log_maxburst
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
        logger->last_logged_time = get_millisecond();
    }
    return SUCCESS;
}

/* 基础日志方法
*/
static int base_log(rasp_logger_entry *logger, severity_level level_int, char *message, int message_len)
{
    if (!logger->accessable)
    {
        return FAILURE;
    }

    if (check_log_level(logger, level_int) == FAILURE)
    {
        return FAILURE;
    }
    
    if (comsume_token_if_available(logger) == FAILURE)
    {
        return FAILURE;
    }

    if (OPENRASP_LOG_G(in_request_process))
    {
        if (logger->appender & FILE_APPENDER)
        {
            openrasp_log_by_file(logger, message, message_len);
        }
        if (logger->appender & SYSLOG_APPENDER)
        {
            openrasp_log_by_syslog(logger, level_int, message, message_len);
        }
    }
    else
    {
        if (logger->appender & FILE_APPENDER)
        {
            char *file_path = NULL;
            zend_string *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), (long)time(NULL), 1);
            spprintf(&file_path, 0, "%s%clogs%c%s%c%s.log.%s", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH, 
                logger->name, DEFAULT_SLASH, logger->name, ZSTR_VAL(tmp_formatted_date_suffix));
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
            zend_string_release(tmp_formatted_date_suffix);
        }
    }

    return SUCCESS;
}

/* 基础info
*/
static inline int base_info(rasp_logger_entry *logger, const char *message, int message_len) {
    return base_log(logger, LEVEL_INFO, (char*)message, message_len);
}

/* 基础debug
*/
static inline int base_debug(rasp_logger_entry *logger, const char *message, int message_len) {
    return base_log(logger, LEVEL_DEBUG, (char*)message, message_len);
}

/*************************************************************************************************/

/* 用于openrasp的info
*/
int rasp_info(const char *message, int message_len) {
    if (!OPENRASP_LOG_G(in_request_process))
    {
        init_logger_instance(RASP_LOGGER);
    }
    char *rasp_info = NULL;    
    zend_string *time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1);
    int   rasp_info_len = spprintf(&rasp_info, 0, "%s %s\n", ZSTR_VAL(time_RFC3339), message);
    int  rasp_result = base_info(&OPENRASP_LOG_G(loggers)[RASP_LOGGER], rasp_info, rasp_info_len);
    zend_string_release(time_RFC3339);
    efree(rasp_info);
    return rasp_result;
}

/* 用于插件的info
*/
int plugin_info(const char *message, int message_len) {
    if (!OPENRASP_LOG_G(in_request_process))
    {
        init_logger_instance(PLUGIN_LOGGER);
    }
    char *plugin_info = NULL;    
    zend_string *time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1);
    int  plugin_info_len = spprintf(&plugin_info, 0, "%s %s\n", ZSTR_VAL(time_RFC3339), message);
    int  plugin_result = base_info(&OPENRASP_LOG_G(loggers)[PLUGIN_LOGGER], plugin_info, plugin_info_len);
    zend_string_release(time_RFC3339);
    efree(plugin_info);
    return plugin_result;
}

/* 用于报警的info
*/
int alarm_info(zval *params_result) {
    assert(Z_TYPE_P(params_result) == IS_ARRAY);
    assert(OPENRASP_LOG_G(in_request_process));

    int alarm_result = FAILURE;

    add_assoc_str(&OPENRASP_LOG_G(alarm_request_info), "event_time", php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1));
    zval trace;
    format_debug_backtrace_str(&trace);
    add_assoc_zval(&OPENRASP_LOG_G(alarm_request_info), "stack_trace", &trace);

    if (php_array_merge(Z_ARRVAL(OPENRASP_LOG_G(alarm_request_info)), Z_ARRVAL_P(params_result)))
    {
        smart_str buf_json = {0};
        php_json_encode(&buf_json, &OPENRASP_LOG_G(alarm_request_info), 0);
        smart_str_appendc(&buf_json, '\n');
        smart_str_0(&buf_json);
        alarm_result = base_info(&OPENRASP_LOG_G(loggers)[ALARM_LOGGER], ZSTR_VAL(buf_json.s), ZSTR_LEN(buf_json.s));
        smart_str_free(&buf_json);
        delete_merged_array_keys(Z_ARRVAL(OPENRASP_LOG_G(alarm_request_info)), Z_ARRVAL_P(params_result));
    }
    else
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("Fail to merge request parameters during alarm logging."));
    }

    zend_hash_str_del(Z_ARRVAL(OPENRASP_LOG_G(alarm_request_info)), ZEND_STRL("stack_trace"));
    zend_hash_str_del(Z_ARRVAL(OPENRASP_LOG_G(alarm_request_info)), ZEND_STRL("event_time"));

    return alarm_result;
}

/* 用于基线的info
*/
int policy_info(zval *params_result) {
    assert(Z_TYPE_P(params_result) == IS_ARRAY);

    if (!OPENRASP_LOG_G(in_request_process))
    {
        init_logger_instance(POLICY_LOGGER);
        init_policy_request_info();
    }
    
    int policy_result = FAILURE;

    add_assoc_str(&OPENRASP_LOG_G(policy_request_info), "event_time", php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1));
    zval trace;
    if (OPENRASP_LOG_G(in_request_process))
    {
        format_debug_backtrace_str(&trace);
    }
    else
    {
         ZVAL_STRING(&trace, "");
    }
    add_assoc_zval(&OPENRASP_LOG_G(policy_request_info), "stack_trace", &trace);

    if (php_array_merge(Z_ARRVAL(OPENRASP_LOG_G(policy_request_info)), Z_ARRVAL_P(params_result)))
    {
        smart_str buf_json = {0};      
        php_json_encode(&buf_json, &OPENRASP_LOG_G(policy_request_info), 0);
        smart_str_appendc(&buf_json, '\n');
        smart_str_0(&buf_json);
        policy_result = base_info(&OPENRASP_LOG_G(loggers)[POLICY_LOGGER], ZSTR_VAL(buf_json.s), ZSTR_LEN(buf_json.s));
        smart_str_free(&buf_json);
        delete_merged_array_keys(Z_ARRVAL(OPENRASP_LOG_G(policy_request_info)), Z_ARRVAL_P(params_result));
    }
    else
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("Fail to merge request parameters during policy logging."));
    }

    zend_hash_str_del(Z_ARRVAL(OPENRASP_LOG_G(policy_request_info)), ZEND_STRL("stack_trace"));
    zend_hash_str_del(Z_ARRVAL(OPENRASP_LOG_G(policy_request_info)), ZEND_STRL("event_time"));

    if (!OPENRASP_LOG_G(in_request_process))
    {                
        clear_policy_request_info();
    }

    return policy_result;
}

static void openrasp_log_init_globals(zend_openrasp_log_globals *openrasp_log_globals)
{
    ZVAL_NULL(&openrasp_log_globals->alarm_request_info);
    ZVAL_NULL(&openrasp_log_globals->policy_request_info);

	openrasp_log_globals->formatted_date_suffix = nullptr;	
	openrasp_log_globals->syslog_stream = nullptr;
    openrasp_log_globals->in_request_process = 0;
    openrasp_log_globals->last_retry_time = 0;

    openrasp_log_globals->loggers[ALARM_LOGGER] = {"alarm",  0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[POLICY_LOGGER] = {"policy", 0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[PLUGIN_LOGGER] = {"plugin", 0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};
    openrasp_log_globals->loggers[RASP_LOGGER] = {"rasp",   0, 0, 0l, 0, LEVEL_INFO, FILE_APPENDER, nullptr};

    log_appender alarm_appender = FILE_APPENDER;
    if (openrasp_ini.syslog_alarm_enable && openrasp_ini.syslog_server_address) {
        php_url *resource = php_url_parse_ex(openrasp_ini.syslog_server_address, strlen(openrasp_ini.syslog_server_address));
        if (resource) {
            if (resource->scheme && (!strcmp(resource->scheme, "tcp") || !strcmp(resource->scheme, "udp"))) {
                alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
            }
            else
            {
                openrasp_error(E_WARNING, LOG_ERROR, 
                _("Invalid url scheme in syslog server address: '%s', expecting 'tcp:' or 'udp:'."), openrasp_ini.syslog_server_address);
            }            
            php_url_free(resource);
        } else {
            openrasp_error(E_WARNING, LOG_ERROR, 
                _("Invalid syslog server address: '%s', expecting 'tcp://' or 'udp://' to be present."), openrasp_ini.syslog_server_address);
        }
    }
    openrasp_log_globals->loggers[ALARM_LOGGER].appender = alarm_appender;
}

PHP_MINIT_FUNCTION(openrasp_log)
{
	ZEND_INIT_MODULE_GLOBALS(openrasp_log, openrasp_log_init_globals, NULL);
    openrasp_shared_alloc_startup();
#if defined(PHP_WIN32) && defined(HAVE_IPHLPAPI_WS2)
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        openrasp_error(E_WARNING, LOG_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            openrasp_error(E_WARNING, LOG_ERROR, _("Error allocating memory needed to call GetAdaptersinfo."));
        }
    }
    if (pAdapterInfo != NULL && (dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            _if_addr_map.insert(std::pair<std::string, std::string>(pAdapter->Description, pAdapter->IpAddressList.IpAddress.String));
            pAdapter = pAdapter->Next;
        }
        FREE(pAdapterInfo);
    }
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) 
    {
        openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs error: %s"), strerror(errno));
    }
    else
    {
        int n, s;
        char host[NI_MAXHOST];
        for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
        {
            if (ifa->ifa_addr == NULL)
            {
                continue;
            }
            if ((strcmp("lo", ifa->ifa_name) == 0) ||
            !(ifa->ifa_flags & (IFF_RUNNING)))
            {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                    openrasp_error(E_WARNING, LOG_ERROR, _("getifaddrs error: getnameinfo failed - %s."), gai_strerror(s));
                }
                _if_addr_map.insert(std::pair<std::string, std::string>(ifa->ifa_name, host));
            }
        }
        freeifaddrs(ifaddr);
    }
#endif
    if (gethostname(host_name, sizeof(host_name) - 1)) { 
        sprintf( host_name, "UNKNOWN_HOST" );
        openrasp_error(E_WARNING, LOG_ERROR, _("gethostname error: %s"), strerror(errno));
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_log)
{
    openrasp_shared_alloc_shutdown();
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_log)
{
    OPENRASP_LOG_G(in_request_process) = 1;
	long now = (long)time(NULL);
    OPENRASP_LOG_G(formatted_date_suffix) = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now, 1);
    init_openrasp_loggers();
    init_alarm_request_info();
    init_policy_request_info();
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(openrasp_log)
{
    assert(OPENRASP_LOG_G(formatted_date_suffix) != nullptr);
    zend_string_release(OPENRASP_LOG_G(formatted_date_suffix));
    OPENRASP_LOG_G(formatted_date_suffix) = nullptr;
    
    if (OPENRASP_LOG_G(syslog_stream))
    {
        php_stream_close(OPENRASP_LOG_G(syslog_stream));        
        OPENRASP_LOG_G(syslog_stream) = nullptr;
    }
    clear_openrasp_loggers();
    clear_alarm_request_info();
    clear_policy_request_info();
    OPENRASP_LOG_G(in_request_process) = 0;
    return SUCCESS;
}
