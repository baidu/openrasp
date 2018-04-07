#ifndef OPENRASP_ARRAY_H
#define OPENRASP_ARRAY_H

#include "php.h"
#include "php_ini.h"
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#ifdef PHP_WIN32
#include "win32/unistd.h"
#endif
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "php_globals.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_rand.h"
#include "ext/standard/php_smart_str.h"

/* {{{ defines */
#define EXTR_OVERWRITE			0
#define EXTR_SKIP				1
#define EXTR_PREFIX_SAME		2
#define	EXTR_PREFIX_ALL			3
#define	EXTR_PREFIX_INVALID		4
#define	EXTR_PREFIX_IF_EXISTS	5
#define	EXTR_IF_EXISTS			6

#define EXTR_REFS				0x100

#define CASE_LOWER				0
#define CASE_UPPER				1

#define DIFF_NORMAL			1
#define DIFF_KEY			2
#define DIFF_ASSOC			6
#define DIFF_COMP_DATA_NONE    -1
#define DIFF_COMP_DATA_INTERNAL 0
#define DIFF_COMP_DATA_USER     1
#define DIFF_COMP_KEY_INTERNAL  0
#define DIFF_COMP_KEY_USER      1

#define INTERSECT_NORMAL		1
#define INTERSECT_KEY			2
#define INTERSECT_ASSOC			6
#define INTERSECT_COMP_DATA_NONE    -1
#define INTERSECT_COMP_DATA_INTERNAL 0
#define INTERSECT_COMP_DATA_USER     1
#define INTERSECT_COMP_KEY_INTERNAL  0
#define INTERSECT_COMP_KEY_USER      1

#define DOUBLE_DRIFT_FIX	0.000000000000001
/* }}} */

void pre_global_array_diff_ukey(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_array_filter(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_array_map(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_array_walk(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_uasort(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_uksort(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_usort(INTERNAL_FUNCTION_PARAMETERS);

void pre_reflectionfunction___construct(INTERNAL_FUNCTION_PARAMETERS);

#endif //OPENRASP_ARRAY_H