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

#ifndef OPENRASP_HOOK_H
#define OPENRASP_HOOK_H

#include "openrasp.h"
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "php_output.h"
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "php_globals.h"
#include "ext/standard/file.h"
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_API.h"
#include "Zend/zend_compile.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/php_rand.h"
#include "ext/standard/php_smart_str.h"
#ifdef PHP_WIN32
#include "win32/unistd.h"
#endif
#ifdef __cplusplus
}
#endif
#include <string>
#include <set>

#ifdef ZEND_WIN32
# ifndef MAXPATHLEN
#  define MAXPATHLEN     _MAX_PATH
# endif
#else
# ifndef MAXPATHLEN
#  define MAXPATHLEN     4096
# endif
#endif

#define OPENRASP_INTERNAL_FUNCTION_PARAMETERS INTERNAL_FUNCTION_PARAMETERS, const char * check_type
#define OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU INTERNAL_FUNCTION_PARAM_PASSTHRU, check_type

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

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
#define OPENRASP_OP1_TYPE(n) ((n)->op1.op_type)
#define OPENRASP_OP2_TYPE(n) ((n)->op2.op_type)
#define OPENRASP_OP1_VAR(n) ((n)->op1.u.var)
#define OPENRASP_OP2_VAR(n) ((n)->op2.u.var)
#define OPENRASP_OP1_CONSTANT_PTR(n) (&(n)->op1.u.constant)
#define OPENRASP_OP2_CONSTANT_PTR(n) (&(n)->op2.u.constant)
#define OPENRASP_INCLUDE_OR_EVAL_TYPE(n) (Z_LVAL(n->op2.u.constant))
#else
#define OPENRASP_OP1_TYPE(n) ((n)->op1_type)
#define OPENRASP_OP2_TYPE(n) ((n)->op2_type)
#define OPENRASP_OP1_VAR(n) ((n)->op1.var)
#define OPENRASP_OP2_VAR(n) ((n)->op2.var)
#define OPENRASP_OP1_CONSTANT_PTR(n) ((n)->op1.zv)
#define OPENRASP_OP2_CONSTANT_PTR(n) ((n)->op2.zv)
#define OPENRASP_INCLUDE_OR_EVAL_TYPE(n) (n->extended_value)
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 5)
#define OPENRASP_T(offset) (*(temp_variable *)((char *)execute_data->Ts + offset))
#define OPENRASP_CV(i) (execute_data->CVs[i])
#define OPENRASP_CV_OF(i) (EG(current_execute_data)->CVs[i])
#else
#define OPENRASP_T(offset) (*EX_TMP_VAR(execute_data, offset))
#define OPENRASP_CV(i) (*EX_CV_NUM(execute_data, i))
#define OPENRASP_CV_OF(i) (*EX_CV_NUM(EG(current_execute_data), i))
#endif

#define MYSQLI_STORE_RESULT 0
#define MYSQLI_USE_RESULT 	1
#define MYSQL_PORT          3306

typedef struct sql_connection_entry_t {
	char *server = nullptr;
    char *host = nullptr;
    int   port = 0;
    char *username = nullptr;
} sql_connection_entry;

typedef enum wrapper_operation_t {
    OPENDIR         = 1 << 0,
    RENAMESRC       = 1 << 1,
    RENAMEDEST      = 1 << 2,
	READING         = 1 << 3,
    WRITING         = 1 << 4,
    APPENDING       = 1 << 5,
    SIMULTANEOUSRW  = 1 << 6
} wrapper_operation;

typedef void (*init_connection_t)(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p);
typedef void (*hook_handler_t)(TSRMLS_D);

void slow_query_alarm(int rows TSRMLS_DC);
zend_bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func, int enforce_policy);
void sql_type_handler(char* query, int query_len, char *server TSRMLS_DC);
long fetch_rows_via_user_function(const char *f_name_str, zend_uint param_count, zval *params[] TSRMLS_DC);

typedef enum hook_position_t {
	PRE_HOOK = 1 << 0, 
    POST_HOOK  = 1 << 1
} hook_position;

typedef void (*php_function)(INTERNAL_FUNCTION_PARAMETERS);
/**
 * 使用这个宏定义被 hook 函数的替换函数的函数头部
 * 在函数体的适当位置添加 origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU); 可继续执行原始函数
 * 执行原始函数前，可调用 zend_get_parameters 等函数获取参数信息
 * 执行原始函数后，可查看 return_value 变量获取返回信息
 * 
 * @param name 函数完整名称
 * @param scope 函数所属 class，全局函数的 scope 为 global
 */
#define DEFINE_HOOK_HANDLER_EX(name, scope, type)                                                          \
    {                                                                                                      \
        HashTable *ht = nullptr;                                                                           \
        zend_function *function;                                                                           \
        if (strcmp("global", ZEND_TOSTR(scope)) == 0)                                                      \
        {                                                                                                  \
            ht = CG(function_table);                                                                       \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            zend_class_entry **clazz;                                                                      \
            if (zend_hash_find(CG(class_table), ZEND_STRS(ZEND_TOSTR(scope)), (void **)&clazz) == SUCCESS) \
            {                                                                                              \
                ht = &(*clazz)->function_table;                                                            \
            }                                                                                              \
        }                                                                                                  \
        if (ht &&                                                                                          \
            zend_hash_find(ht, ZEND_STRS(ZEND_TOSTR(name)), (void **)&function) == SUCCESS &&              \
            function->internal_function.handler != zif_display_disabled_function)                          \
        {                                                                                                  \
            origin_##scope##_##name##_##type = function->internal_function.handler;                        \
            function->internal_function.handler = hook_##scope##_##name##_##type;                          \
        }                                                                                                  \
    }
    
#define OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                                                \
    php_function origin_##scope##_##name##_##type = nullptr;                                                        \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function);    \
    void hook_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAMETERS)                                               \
    {                                                                                                               \
        hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, origin_##scope##_##name##_##type);    \
    }                                                                                                               \
    void scope##_##name##_##type##_handler(TSRMLS_D)                                                                \
    DEFINE_HOOK_HANDLER_EX(name, scope, type)                                                                       \
    int scope##_##name##_##type = [](){register_hook_handler(scope##_##name##_##type##_handler);return 0;}();       \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function)

#define OPENRASP_HOOK_FUNCTION(name, type) \
    OPENRASP_HOOK_FUNCTION_EX(name, global, type)

#define HOOK_FUNCTION_EX(name, scope, type)                                                            \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);                         \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);                        \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                                       \
    {                                                                                                  \
        static bool type_ignored = openrasp_check_type_ignored(ZEND_STRL(ZEND_TOSTR(type)) TSRMLS_CC); \
        if (UNLIKELY(type_ignored))                                                                    \
        {                                                                                              \
            return origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                  \
        }                                                                                              \
        pre_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (ZEND_TOSTR(type)));           \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                             \
        post_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (ZEND_TOSTR(type)));          \
    }

#define HOOK_FUNCTION(name, type) \
    HOOK_FUNCTION_EX(name, global, type)

#define PRE_HOOK_FUNCTION_EX(name, scope, type)                                                        \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);                         \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                                       \
    {                                                                                                  \
        static bool type_ignored = openrasp_check_type_ignored(ZEND_STRL(ZEND_TOSTR(type)) TSRMLS_CC); \
        if (LIKELY(!type_ignored))                                                                     \
        {                                                                                              \
            pre_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (ZEND_TOSTR(type)));       \
        }                                                                                              \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                             \
    }

#define PRE_HOOK_FUNCTION(name, type) \
    PRE_HOOK_FUNCTION_EX(name, global, type)

#define POST_HOOK_FUNCTION_EX(name, scope, type)                                                       \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);                        \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                                       \
    {                                                                                                  \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                             \
        static bool type_ignored = openrasp_check_type_ignored(ZEND_STRL(ZEND_TOSTR(type)) TSRMLS_CC); \
        if (LIKELY(!type_ignored))                                                                     \
        {                                                                                              \
            post_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (ZEND_TOSTR(type)));      \
        }                                                                                              \
    }

#define POST_HOOK_FUNCTION(name, type) \
    POST_HOOK_FUNCTION_EX(name, global, type)

struct openrasp_hook_ini_t
{
    unsigned int slowquery_min_rows = 500;
    bool enforce_policy = false;
    char *block_url = nullptr;
    std::set<std::string> hooks_ignore;
    std::set<std::string> callable_blacklists;//haha
};
extern struct openrasp_hook_ini_t openrasp_hook_ini;

ZEND_BEGIN_MODULE_GLOBALS(openrasp_hook)

ZEND_END_MODULE_GLOBALS(openrasp_hook)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_hook);

#ifdef ZTS
#define OPENRASP_HOOK_G(v) TSRMG(openrasp_hook_globals_id, zend_openrasp_hook_globals *, v)
#define OPENRASP_HOOK_GP() ((zend_openrasp_hook_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_hook_globals_id)])
#else
#define OPENRASP_HOOK_G(v) (openrasp_hook_globals.v)
#define OPENRASP_HOOK_GP() (&openrasp_hook_globals)
#endif

PHP_MINIT_FUNCTION(openrasp_hook);
PHP_MSHUTDOWN_FUNCTION(openrasp_hook);
PHP_RINIT_FUNCTION(openrasp_hook);
PHP_RSHUTDOWN_FUNCTION(openrasp_hook);

typedef void (*fill_param_t)(HashTable *ht);

void handle_block(TSRMLS_D);
void check(const char *type, zval *params TSRMLS_DC);
bool openrasp_check_type_ignored(const char *item_name, uint item_name_length TSRMLS_DC);
bool openrasp_check_callable_black(const char *item_name, uint item_name_length TSRMLS_DC);
bool openrasp_zval_in_request(zval *item TSRMLS_DC);
void openrasp_buildin_php_risk_handle(zend_bool is_block, const char *type, int confidence, zval *params, zval *message TSRMLS_DC);
char * openrasp_real_path(char *filename, int filename_len, zend_bool use_include_path, wrapper_operation w_op TSRMLS_DC);
void register_hook_handler(hook_handler_t hook_handler);

#endif