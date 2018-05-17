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

extern "C" {
#include "openrasp_shared_alloc.h"
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/microtime.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_smart_str.h"
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

#define INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(original_name, new_name, p_zval) do {              \
    zval **new_name##_item;                                                                         \
    if (Z_TYPE_P(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY ||                                \
        zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]),                             \
        ZEND_STRS(ZEND_TOSTR(original_name)),                                                       \
        (void **)&new_name##_item) == FAILURE || Z_TYPE_PP(new_name##_item) != IS_STRING)           \
        {                                                                                           \
            add_assoc_string(p_zval, ZEND_TOSTR(new_name), "", 1);                                  \
        } else {                                                                                    \
            add_assoc_zval(p_zval, ZEND_TOSTR(new_name), *new_name##_item);                         \
            Z_ADDREF_PP(new_name##_item);                                                           \
        }                                                                                           \
} while(0)

#define INIT_LOGGER_IF_NEED(logger_prefix) do {                                                     \
    if (!OPENRASP_LOG_G(logger_prefix##_logger).initialized)                                        \
    {                                                                                               \
        OPENRASP_LOG_G(logger_prefix##_logger).available_token   = openrasp_ini.log_maxburst;       \
        OPENRASP_LOG_G(logger_prefix##_logger).last_logged_time  = get_millisecond(TSRMLS_C);       \
        char *logger_prefix##_folder = NULL;                                                        \
        spprintf(&logger_prefix##_folder, 0, "%s%clogs%c%s", openrasp_ini.root_dir, DEFAULT_SLASH,  \
        DEFAULT_SLASH, OPENRASP_LOG_G(logger_prefix##_logger).name);                                \
        OPENRASP_LOG_G(logger_prefix##_logger).accessable                                           \
        = (openrasp_log_files_mkdir(logger_prefix##_folder TSRMLS_CC) == SUCCESS) ? 1 : 0;          \
        efree(logger_prefix##_folder);                                                              \
        OPENRASP_LOG_G(logger_prefix##_logger).initialized       = 1;                               \
    }                                                                                               \
} while(0)

#define CLEAR_LOGGER_STREAM(logger_prefix) do{                                                      \
    if (OPENRASP_LOG_G(logger_prefix##_logger).stream_log)                                          \
    {                                                                                               \
        php_stream_close(OPENRASP_LOG_G(logger_prefix##_logger).stream_log);                        \
        OPENRASP_LOG_G(logger_prefix##_logger).stream_log = NULL;                                   \
    }                                                                                               \
} while(0)

#define DELETE_MERGED_ARRAY_KEYS(dest, src) do {                                                    \
    HashTable *hash_##src = Z_ARRVAL_P(src);                                                        \
    for(zend_hash_internal_pointer_reset(hash_##src);                                               \
    zend_hash_has_more_elements(hash_##src) == SUCCESS;                                             \
    zend_hash_move_forward(hash_##src)){                                                            \
        char *key;                                                                                  \
        uint keylen;                                                                                \
        ulong idx;                                                                                  \
        int type = zend_hash_get_current_key_ex(hash_##src, &key, &keylen, &idx, 0, NULL);          \
        if (type == HASH_KEY_IS_STRING)                                                             \
        {                                                                                           \
            zend_hash_del(dest, key, keylen);                                                       \
        }                                                                                           \
    }                                                                                               \
} while(0)

static std::map<std::string, std::string> _if_addr_map;
static char host_name[255];

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

static void init_alarm_request_info(TSRMLS_D)
{
    assert(OPENRASP_LOG_G(alarm_request_info) == nullptr);
    MAKE_STD_ZVAL(OPENRASP_LOG_G(alarm_request_info));
    array_init(OPENRASP_LOG_G(alarm_request_info));

    if (!PG(http_globals)[TRACK_VARS_SERVER] && !zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
    {
        return;
    }

    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(REMOTE_ADDR,        attack_source,  OPENRASP_LOG_G(alarm_request_info));
    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(SERVER_NAME,        target,         OPENRASP_LOG_G(alarm_request_info));
    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(SERVER_ADDR,        server_ip,      OPENRASP_LOG_G(alarm_request_info));
    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(REQUEST_URI,        url,            OPENRASP_LOG_G(alarm_request_info));
    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(HTTP_REFERER,       referer,        OPENRASP_LOG_G(alarm_request_info));
    INIT_REQUEST_INFO_FROM_TRACK_VARS_SERVER(HTTP_USER_AGENT,    user_agent,     OPENRASP_LOG_G(alarm_request_info));

    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "event_type", "attack", 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_hostname", host_name, 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_type", "PHP", 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "server_version", OPENRASP_PHP_VERSION, 1);
    add_assoc_string(OPENRASP_LOG_G(alarm_request_info), "request_id", OPENRASP_INJECT_G(request_id), 1);
    
    zval **info_item;
    zval *path = NULL;        
    static const char REQUEST_URI[] = "REQUEST_URI";                                                                                                                                                                       
    if (Z_TYPE_P(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY 
        && zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRS(REQUEST_URI),     
        (void **)&info_item) == SUCCESS && Z_TYPE_PP(info_item) == IS_STRING) 
        {
            char *haystack = Z_STRVAL_PP(info_item);                                            
            int   haystack_len = Z_STRLEN_PP(info_item);                
            const char *found = php_memnstr(haystack, "?", 1, haystack + haystack_len);
            if (found)
            {
                add_assoc_stringl(OPENRASP_LOG_G(alarm_request_info), "path", haystack, found - haystack, 1);
            }
        }
}

static void init_policy_request_info(TSRMLS_D)
{
    if (OPENRASP_LOG_G(policy_request_info))
    {
        return;
    }
    MAKE_STD_ZVAL(OPENRASP_LOG_G(policy_request_info));
    array_init(OPENRASP_LOG_G(policy_request_info));
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "event_type", "security_policy", 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_hostname", host_name, 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_type", "PHP", 1);
    add_assoc_string(OPENRASP_LOG_G(policy_request_info), "server_version", OPENRASP_PHP_VERSION, 1);
    add_assoc_zval(OPENRASP_LOG_G(policy_request_info), "server_nic", _get_ifaddr_zval());
}

static void clear_alarm_request_info(TSRMLS_D)
{
    if (OPENRASP_LOG_G(alarm_request_info))
    {
        zval_ptr_dtor(&OPENRASP_LOG_G(alarm_request_info));
        OPENRASP_LOG_G(alarm_request_info) = NULL;
    }
}

static void clear_policy_request_info(TSRMLS_D)
{
    if (OPENRASP_LOG_G(policy_request_info))
    {
        zval_ptr_dtor(&OPENRASP_LOG_G(policy_request_info));
        OPENRASP_LOG_G(policy_request_info) = NULL;
    }
}

/* 初始化logger
*/
static void init_openrasp_loggers(TSRMLS_D)
{    
    INIT_LOGGER_IF_NEED(rasp);
    INIT_LOGGER_IF_NEED(alarm);
    INIT_LOGGER_IF_NEED(plugin);
    INIT_LOGGER_IF_NEED(policy);
}

/* 释放全局logger
*/
static void clear_openrasp_loggers(TSRMLS_D)
{
    CLEAR_LOGGER_STREAM(rasp);
    CLEAR_LOGGER_STREAM(alarm);
    CLEAR_LOGGER_STREAM(plugin);
    CLEAR_LOGGER_STREAM(policy);
}

/* 判断是否需要更新文件时间后缀
*/
static zend_bool if_need_update_formatted_file_suffix(rasp_logger_entry *logger, long now, int log_info_len TSRMLS_DC)
{
    int  last_logged_second       = logger->last_logged_time / 1000;
    long log_rotate_second        = 24*60*60;
    if (now/log_rotate_second != last_logged_second/log_rotate_second)
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
        OPENRASP_LOG_G(formatted_date_suffix) = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now, 1 TSRMLS_CC);
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
    time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), now, 1 TSRMLS_CC);
    priority = openrasp_ini.syslog_facility * 8 + level_int;
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
            char *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), (long)time(NULL), 1 TSRMLS_CC);
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
static inline int base_info(rasp_logger_entry *logger, const char *message, int message_len TSRMLS_DC) {
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
        INIT_LOGGER_IF_NEED(rasp);
    }
    char *rasp_info                 = NULL;    
    char *time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1 TSRMLS_CC);
    int   rasp_info_len = spprintf(&rasp_info, 0, "%s %s\n", time_RFC3339, message);
    int  rasp_result = base_info(&OPENRASP_LOG_G(rasp_logger), rasp_info, rasp_info_len TSRMLS_CC);
    efree(time_RFC3339);
    efree(rasp_info);
    return rasp_result;
}

/* 用于插件的info
*/
int plugin_info(const char *message, int message_len TSRMLS_DC) {
    if (!OPENRASP_LOG_G(in_request_process))
    {        
        INIT_LOGGER_IF_NEED(plugin);
    }
    char *plugin_info                 = NULL;    
    char *time_RFC3339 = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1 TSRMLS_CC);
    int  plugin_info_len = spprintf(&plugin_info, 0, "%s %s\n", time_RFC3339, message);
    int  plugin_result = base_info(&OPENRASP_LOG_G(plugin_logger), plugin_info, plugin_info_len TSRMLS_CC);
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
    char *event_time = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1 TSRMLS_CC);
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
        alarm_result = base_info(&OPENRASP_LOG_G(alarm_logger), buf_json.c, buf_json.len TSRMLS_CC);
        smart_str_free(&buf_json);
        
        DELETE_MERGED_ARRAY_KEYS(Z_ARRVAL_P(OPENRASP_LOG_G(alarm_request_info)), params_result);
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
        INIT_LOGGER_IF_NEED(policy);
        init_policy_request_info(TSRMLS_C);
    }
    
    int policy_result = FAILURE;
    char *event_time = php_format_date(ZEND_STRL(RASP_RFC3339_FORMAT), (long)time(NULL), 1 TSRMLS_CC);
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
        policy_result = base_info(&OPENRASP_LOG_G(policy_logger), buf_json.c, buf_json.len TSRMLS_CC);
        smart_str_free(&buf_json);
        DELETE_MERGED_ARRAY_KEYS(Z_ARRVAL_P(OPENRASP_LOG_G(policy_request_info)), params_result);
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

static void openrasp_log_init_globals(zend_openrasp_log_globals *openrasp_log_globals TSRMLS_DC)
{
	openrasp_log_globals->alarm_request_info 				= NULL;
	openrasp_log_globals->policy_request_info 				= NULL;
	openrasp_log_globals->formatted_date_suffix 			= NULL;	
	openrasp_log_globals->syslog_stream       				= NULL;
    openrasp_log_globals->in_request_process                = 0;
    openrasp_log_globals->last_retry_time                   = 0;

	memset(&openrasp_log_globals->rasp_logger, 0, sizeof(rasp_logger_entry));
	openrasp_log_globals->rasp_logger.name     			    = const_cast<char *>(RASP_LOGGER);
	openrasp_log_globals->rasp_logger.level  	   			= LEVEL_INFO;
	openrasp_log_globals->rasp_logger.appender     		    = FILE_APPENDER;

	memset(&openrasp_log_globals->alarm_logger, 0, sizeof(rasp_logger_entry));
	openrasp_log_globals->alarm_logger.name     			= ALARM_LOGGER;
	openrasp_log_globals->alarm_logger.level  	   			= LEVEL_INFO;
    log_appender alarm_appender = FILE_APPENDER;
    if (openrasp_ini.syslog_alarm_enable && openrasp_ini.syslog_server_address) {
        php_url *resource = php_url_parse_ex(openrasp_ini.syslog_server_address, strlen(openrasp_ini.syslog_server_address));
        if (resource) {
            if (resource->scheme && (!strcmp(resource->scheme, "tcp") || !strcmp(resource->scheme, "udp"))) {
                alarm_appender = static_cast<log_appender>(alarm_appender | SYSLOG_APPENDER);
            }
            php_url_free(resource);
        } else {
            openrasp_error(E_WARNING, LOG_ERROR, 
                _("Invalid syslog server address: '%s', expecting 'tcp://' or 'udp://' to be present."), openrasp_ini.syslog_server_address);
        }
    }
    openrasp_log_globals->alarm_logger.appender     		= alarm_appender;

	memset(&openrasp_log_globals->plugin_logger, 0, sizeof(rasp_logger_entry));
	openrasp_log_globals->plugin_logger.name     			= PLUGIN_LOGGER;
	openrasp_log_globals->plugin_logger.level  	   		    = LEVEL_INFO;
	openrasp_log_globals->plugin_logger.appender     		= FILE_APPENDER;
	
	memset(&openrasp_log_globals->policy_logger, 0, sizeof(rasp_logger_entry));
	openrasp_log_globals->policy_logger.name     			= POLICY_LOGGER;
	openrasp_log_globals->policy_logger.level  	   		    = LEVEL_INFO;
	openrasp_log_globals->policy_logger.appender     		= FILE_APPENDER;
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
    OPENRASP_LOG_G(formatted_date_suffix) = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), now, 1 TSRMLS_CC);
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
