#ifndef OPENRASP_H
#define OPENRASP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "php_openrasp.h"

#ifdef HAVE_GETTEXT
#include <libintl.h>
#include <locale.h>
#define GETTEXT_PACKAGE "openrasp"
#define _(STRING) gettext(STRING)
#else
#define _(STRING) (STRING)
#endif

#ifdef __cplusplus
}
#endif

#define FSWATCH_ERROR (20001)
#define LOG_ERROR (20002)
#define SHM_ERROR (20003)
#define CONFIG_ERROR (20004)
#define PLUGIN_ERROR (20005)

#ifndef ZEND_SHUTDOWN_MODULE_GLOBALS
#ifdef ZTS
#define ZEND_SHUTDOWN_MODULE_GLOBALS(module_name, globals_dtor) \
	ts_free_id(module_name##_globals_id);
#else
#define ZEND_SHUTDOWN_MODULE_GLOBALS(module_name, globals_dtor) \
	globals_dtor(&module_name##_globals);
#endif
#endif

ZEND_BEGIN_MODULE_GLOBALS(openrasp)
zend_bool locked;
ZEND_END_MODULE_GLOBALS(openrasp)

ZEND_EXTERN_MODULE_GLOBALS(openrasp)

#ifdef ZTS
#define OPENRASP_G(v) TSRMG(openrasp_globals_id, zend_openrasp_globals *, v)
#define OPENRASP_GP() ((zend_openrasp_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_globals_id)])
#else
#define OPENRASP_G(v) (openrasp_globals.v)
#define OPENRASP_GP() (&openrasp_globals)
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned char openrasp_check(const char *c_type, zval *z_params TSRMLS_DC);
int rasp_info(const char *message, int message_len TSRMLS_DC);
int plugin_info(const char *message, int message_len TSRMLS_DC);
int alarm_info(zval *params_result TSRMLS_DC);
int policy_info(zval *params_result TSRMLS_DC);
void format_debug_backtrace_str(zval *backtrace_str TSRMLS_DC);
void format_debug_backtrace_arr(zval *backtrace_arr TSRMLS_DC);
void openrasp_error(int type, int error_code, const char *format, ...);
int recursive_mkdir(const char *path, int len, int mode TSRMLS_DC);

#ifdef __cplusplus
}
#endif

#endif