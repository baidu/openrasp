#pragma once

#include "openrasp.h"
#include "openrasp_log.h"
extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
}

PHP_MINIT_FUNCTION(openrasp_fswatch);
PHP_MSHUTDOWN_FUNCTION(openrasp_fswatch);