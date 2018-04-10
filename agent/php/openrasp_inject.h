#ifndef OPENRASP_INJECT_H
#define OPENRASP_INJECT_H

#include "openrasp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "ext/standard/php_string.h"
#ifdef __cplusplus
}
#endif

ZEND_BEGIN_MODULE_GLOBALS(openrasp_inject)
char *request_id;
ZEND_END_MODULE_GLOBALS(openrasp_inject)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_inject);

#ifdef ZTS
#define OPENRASP_INJECT_G(v) TSRMG(openrasp_inject_globals_id, zend_openrasp_inject_globals *, v)
#define OPENRASP_INJECT_GP() ((zend_openrasp_inject_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_inject_globals_id)])
#else
#define OPENRASP_INJECT_G(v) (openrasp_inject_globals.v)
#define OPENRASP_INJECT_GP() (&openrasp_inject_globals)
#endif

PHP_MINIT_FUNCTION(openrasp_inject);
PHP_MSHUTDOWN_FUNCTION(openrasp_inject);
PHP_RINIT_FUNCTION(openrasp_inject);
PHP_RSHUTDOWN_FUNCTION(openrasp_inject);


#endif