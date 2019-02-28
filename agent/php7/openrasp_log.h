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

#ifndef OPENRASP_LOG_H
#define OPENRASP_LOG_H

#include "openrasp.h"
#include "agent/shared_log_manager.h"
#include <map>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "php_streams.h"

#ifdef __cplusplus
}
#endif

#define ALARM_LOG_DIR_NAME "alarm"
#define POLICY_LOG_DIR_NAME "policy"
#define PLUGIN_LOG_DIR_NAME "plugin"
#define RASP_LOG_DIR_NAME "rasp"

extern std::unique_ptr<openrasp::SharedLogManager> slm;

typedef enum log_appender_t
{
    NULL_APPENDER = 0,
    FILE_APPENDER = 1 << 0,
    SYSLOG_APPENDER = 1 << 1,
    FSTREAM_APPENDER = 1 << 2
} log_appender;

typedef enum logger_instance_t
{
    ALARM_LOGGER,
    POLICY_LOGGER,
    PLUGIN_LOGGER,
    RASP_LOGGER,
    TOTAL
} logger_instance;

//reference https://en.wikipedia.org/wiki/Syslog
typedef enum severity_level_t
{
    LEVEL_EMERG = 0,
    LEVEL_ALERT = 1,
    LEVEL_CRIT = 2,
    LEVEL_ERR = 3,
    LEVEL_WARNING = 4,
    LEVEL_NOTICE = 5,
    LEVEL_INFO = 6,
    LEVEL_DEBUG = 7
} severity_level;

class RaspLoggerEntry
{
  private:
    const char *name;
    zend_bool initialized = false;
    zend_bool accessable = false;

    int available_token = 0;
    long last_logged_time = 0;

    severity_level level = LEVEL_INFO;
    log_appender appender = NULL_APPENDER;
    log_appender appender_mask = NULL_APPENDER;

    long time_offset;
    char *formatted_date_suffix = nullptr;

    long syslog_reconnect_time;
    zval common_info;
    php_stream *stream_log = nullptr;
    php_stream *syslog_stream = nullptr;

  private:
    void update_common_info();
    void close_streams();
    void clear_common_info();
    void update_formatted_date_suffix();
    void clear_formatted_date_suffix();
    bool comsume_token_if_available();
    bool check_log_level(severity_level level_int) const;
    bool if_need_update_formatted_file_suffix(long now) const;
    bool openrasp_log_stream_available(log_appender appender_int);
    bool raw_log(severity_level level_int, const char *message, int message_len);

  public:
    static const char *default_log_suffix;
    static const char *rasp_rfc3339_format;
    static const char *syslog_time_format;

    RaspLoggerEntry();
    RaspLoggerEntry(const char *name, severity_level level, log_appender appender, log_appender appender_mask);
    RaspLoggerEntry(const RaspLoggerEntry &src) = delete;

    void init(log_appender appender_int);
    void clear();
    bool log(severity_level level_int, const char *message, int message_len, bool separate = true, bool detail = true);
    bool log(severity_level level_int, zval *params_result);
    char *get_formatted_date_suffix() const;
    zval *get_common_info();
    void set_level(severity_level level);

    static std::string level_to_name(severity_level level);
    static int name_to_level(const std::string &name);
    static void inner_error(int type, openrasp_error_code code, const char *format, ...);
};

typedef RaspLoggerEntry rasp_logger_entry;

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(openrasp_log)

zend_bool in_request_process;
RaspLoggerEntry plugin_logger;
RaspLoggerEntry alarm_logger;
RaspLoggerEntry policy_logger;
RaspLoggerEntry rasp_logger;

ZEND_END_MODULE_GLOBALS(openrasp_log)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_log);

#define OPENRASP_LOG_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_log, v)

#define LOG_G(v) OPENRASP_LOG_G(v)

// #ifdef ZTS
// #define OPENRASP_LOG_G(v) TSRMG(openrasp_log_globals_id, zend_openrasp_log_globals *, v)
// #define OPENRASP_LOG_GP() ((zend_openrasp_log_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_log_globals_id)])
// #else
// #define OPENRASP_LOG_G(v) (openrasp_log_globals.v)
// #define OPENRASP_LOG_GP() (&openrasp_log_globals)
// #endif

PHP_MINIT_FUNCTION(openrasp_log);
PHP_MSHUTDOWN_FUNCTION(openrasp_log);
PHP_RINIT_FUNCTION(openrasp_log);
PHP_RSHUTDOWN_FUNCTION(openrasp_log);
PHP_MINFO_FUNCTION(openrasp_log);

bool log_module_initialized();
void update_log_level();
std::map<std::string, std::string> get_if_addr_map();

#endif /* OPENRASP_LOG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
