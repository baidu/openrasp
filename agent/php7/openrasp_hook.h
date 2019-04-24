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

#ifndef OPENRASP_HOOK_H
#define OPENRASP_HOOK_H

#include "openrasp.h"
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_v8.h"
#include "openrasp_utils.h"
#include "openrasp_lru.h"
#include "openrasp_check_type.h"

#ifdef __cplusplus
extern "C"
{
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
#include "zend_types.h"
#include "zend_interfaces.h"
#include "zend_smart_str.h"
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
#ifdef PHP_WIN32
#include "win32/unistd.h"
#endif
#ifdef __cplusplus
}
#endif
#include <string>
#include <set>

#ifdef ZEND_WIN32
#ifndef MAXPATHLEN
#define MAXPATHLEN _MAX_PATH
#endif
#else
#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif
#endif

#define OPENRASP_INTERNAL_FUNCTION_PARAMETERS INTERNAL_FUNCTION_PARAMETERS, OpenRASPCheckType check_type
#define OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU INTERNAL_FUNCTION_PARAM_PASSTHRU, check_type

/* {{{ defines */
#define EXTR_OVERWRITE 0
#define EXTR_SKIP 1
#define EXTR_PREFIX_SAME 2
#define EXTR_PREFIX_ALL 3
#define EXTR_PREFIX_INVALID 4
#define EXTR_PREFIX_IF_EXISTS 5
#define EXTR_IF_EXISTS 6

#define EXTR_REFS 0x100

#define CASE_LOWER 0
#define CASE_UPPER 1

#define DIFF_NORMAL 1
#define DIFF_KEY 2
#define DIFF_ASSOC 6
#define DIFF_COMP_DATA_NONE -1
#define DIFF_COMP_DATA_INTERNAL 0
#define DIFF_COMP_DATA_USER 1
#define DIFF_COMP_KEY_INTERNAL 0
#define DIFF_COMP_KEY_USER 1

#define INTERSECT_NORMAL 1
#define INTERSECT_KEY 2
#define INTERSECT_ASSOC 6
#define INTERSECT_COMP_DATA_NONE -1
#define INTERSECT_COMP_DATA_INTERNAL 0
#define INTERSECT_COMP_DATA_USER 1
#define INTERSECT_COMP_KEY_INTERNAL 0
#define INTERSECT_COMP_KEY_USER 1

#define DOUBLE_DRIFT_FIX 0.000000000000001
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
#define MYSQLI_USE_RESULT 1
#define MYSQL_PORT 3306
#define SAFE_STRING(a) ((a) ? a : "")

typedef enum action_type_t
{
    AC_IGNORE = 0,
    AC_LOG = 1 << 0,
    AC_BLOCK = 1 << 1
} OpenRASPActionType;

enum PATH_OPERATION
{
    OPENDIR = 1 << 0,
    RENAMESRC = 1 << 1,
    RENAMEDEST = 1 << 2,
    READING = 1 << 3,
    WRITING = 1 << 4,
    APPENDING = 1 << 5,
    SIMULTANEOUSRW = 1 << 6
};

typedef void (*hook_handler_t)();
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
#define DEFINE_HOOK_HANDLER_EX(name, scope, type)                                                                                         \
    {                                                                                                                                     \
        HashTable *ht = nullptr;                                                                                                          \
        zend_function *function;                                                                                                          \
        if (strcmp("global", ZEND_TOSTR(scope)) == 0)                                                                                     \
        {                                                                                                                                 \
            ht = CG(function_table);                                                                                                      \
        }                                                                                                                                 \
        else                                                                                                                              \
        {                                                                                                                                 \
            zend_class_entry *clazz;                                                                                                      \
            if ((clazz = static_cast<zend_class_entry *>(zend_hash_str_find_ptr(CG(class_table), ZEND_STRL(ZEND_TOSTR(scope))))) != NULL) \
            {                                                                                                                             \
                ht = &(clazz->function_table);                                                                                            \
            }                                                                                                                             \
        }                                                                                                                                 \
        if (ht &&                                                                                                                         \
            (function = static_cast<zend_function *>(zend_hash_str_find_ptr(ht, ZEND_STRL(ZEND_TOSTR(name))))) != NULL &&                 \
            function->internal_function.handler != zif_display_disabled_function)                                                         \
        {                                                                                                                                 \
            origin_##scope##_##name##_##type = function->internal_function.handler;                                                       \
            function->internal_function.handler = hook_##scope##_##name##_##type;                                                         \
        }                                                                                                                                 \
    }

#define OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                                             \
    php_function origin_##scope##_##name##_##type = nullptr;                                                     \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function); \
    void hook_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAMETERS)                                            \
    {                                                                                                            \
        hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, origin_##scope##_##name##_##type); \
    }                                                                                                            \
    void scope##_##name##_##type##_handler()                                                                     \
        DEFINE_HOOK_HANDLER_EX(name, scope, type) int scope##_##name##_##type = []() {register_hook_handler(scope##_##name##_##type##_handler);return 0; }();                      \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function)

#define OPENRASP_HOOK_FUNCTION(name, type) \
    OPENRASP_HOOK_FUNCTION_EX(name, global, type)

#define HOOK_FUNCTION_EX(name, scope, type)                                           \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);        \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                      \
    {                                                                                 \
        bool pre_type_ignored = openrasp_check_type_ignored(type);                    \
        if (!pre_type_ignored)                                                        \
        {                                                                             \
            pre_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type));  \
        }                                                                             \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                            \
        bool post_type_ignored = openrasp_check_type_ignored(type);                   \
        if (!post_type_ignored)                                                       \
        {                                                                             \
            post_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
        }                                                                             \
    }

#define HOOK_FUNCTION(name, type) \
    HOOK_FUNCTION_EX(name, global, type)

#define PRE_HOOK_FUNCTION_EX(name, scope, type)                                      \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                     \
    {                                                                                \
        bool type_ignored = openrasp_check_type_ignored(type);                       \
        if (!type_ignored)                                                           \
        {                                                                            \
            pre_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
        }                                                                            \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                           \
    }

#define PRE_HOOK_FUNCTION(name, type) \
    PRE_HOOK_FUNCTION_EX(name, global, type)

#define POST_HOOK_FUNCTION_EX(name, scope, type)                                      \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_EX(name, scope, type)                                      \
    {                                                                                 \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                            \
        bool type_ignored = openrasp_check_type_ignored(type);                        \
        if (!type_ignored)                                                            \
        {                                                                             \
            post_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
        }                                                                             \
    }

#define POST_HOOK_FUNCTION(name, type) \
    POST_HOOK_FUNCTION_EX(name, global, type)

ZEND_BEGIN_MODULE_GLOBALS(openrasp_hook)
int check_type_white_bit_mask;
openrasp::LRU<std::string, bool> lru;
ZEND_END_MODULE_GLOBALS(openrasp_hook)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_hook);

#define OPENRASP_HOOK_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_hook, v)

// #ifdef ZTS
// #define OPENRASP_HOOK_G(v) TSRMG(openrasp_hook_globals_id, zend_openrasp_hook_globals *, v)
// #define OPENRASP_HOOK_GP() ((zend_openrasp_hook_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_hook_globals_id)])
// #else
// #define OPENRASP_HOOK_G(v) (openrasp_hook_globals.v)
// #define OPENRASP_HOOK_GP() (&openrasp_hook_globals)
// #endif

PHP_MINIT_FUNCTION(openrasp_hook);
PHP_MSHUTDOWN_FUNCTION(openrasp_hook);
PHP_RINIT_FUNCTION(openrasp_hook);
PHP_RSHUTDOWN_FUNCTION(openrasp_hook);

typedef void (*fill_param_t)(HashTable *ht);

std::string openrasp_real_path(char *filename, int length, bool use_include_path, uint32_t w_op);

void register_hook_handler(hook_handler_t hook_handler);

const std::string get_check_type_name(OpenRASPCheckType type);

bool openrasp_zval_in_request(zval *item);
std::string fetch_name_in_request(zval *item, std::string &var_type);
bool openrasp_check_type_ignored(OpenRASPCheckType check_type);
bool openrasp_check_callable_black(const char *item_name, uint item_name_length);

void openrasp_buildin_php_risk_handle(OpenRASPActionType action, OpenRASPCheckType type, int confidence, zval *params, zval *message);
void handle_block();
void reset_response();

OpenRASPActionType string_to_action(std::string action_string);
std::string action_to_string(OpenRASPActionType type);

#endif