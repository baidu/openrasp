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

#ifndef OPENRASP_LOG_H
#define OPENRASP_LOG_H

#include "openrasp.h"

#ifdef __cplusplus
extern "C" {
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

#define DEFAULT_LOG_FILE_SUFFIX             "%Y-%m-%d"
#define RASP_RFC3339_FORMAT                 "%Y-%m-%dT%H:%M:%S%z"

typedef enum log_appender_t {
	  FILE_APPENDER = 1 << 0, 
    SYSLOG_APPENDER  = 1 << 1
} log_appender;

typedef enum logger_instance_t {
    ALARM_LOGGER,
    POLICY_LOGGER,
    PLUGIN_LOGGER,
    RASP_LOGGER,
    TOTAL
} logger_instance;

//reference https://en.wikipedia.org/wiki/Syslog
typedef enum severity_level_t {
  LEVEL_INFO  = 6, 
  LEVEL_DEBUG = 7,
  LEVEL_ALL   = 8
} severity_level;

typedef struct _rasp_logger_entry_t
{
    char           *name;
    zend_bool       accessable;
    int             available_token;
    long            last_logged_time;
    zend_bool       initialized;
    severity_level  level;
    log_appender    appender;
    php_stream     *stream_log;
} rasp_logger_entry;

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(openrasp_log)

zend_string           *formatted_date_suffix;
php_stream            *syslog_stream;
zval                   alarm_request_info;
zval                   policy_request_info;
zend_bool              enable_alarm_syslog;
zend_bool              in_request_process;
long                   last_retry_time;
long                   time_offset;
rasp_logger_entry      loggers[TOTAL];

ZEND_END_MODULE_GLOBALS(openrasp_log)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_log);

#define OPENRASP_LOG_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_log, v)

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

int base_info(rasp_logger_entry *logger, const char *message, int message_len TSRMLS_DC);

#endif /* OPENRASP_LOG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
