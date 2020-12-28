/*
 * Copyright 2017-2021 Baidu Inc.
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
#include "openrasp_ini.h"
#include "openrasp_v8.h"
#include "openrasp_utils.h"
#include "openrasp_lru.h"
#include "openrasp_check_type.h"
#include "utils/string.h"
#include "model/zend_ref_item.h"
#include "utils/double_array_trie.h"

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
#include <unordered_set>
#include <unordered_map>

#ifdef ZEND_WIN32
#ifndef MAXPATHLEN
#define MAXPATHLEN _MAX_PATH
#endif
#else
#ifndef MAXPATHLEN
#define MAXPATHLEN (4096)
#endif
#endif

#define OPENRASP_INTERNAL_FUNCTION_PARAMETERS INTERNAL_FUNCTION_PARAMETERS, OpenRASPCheckType check_type
#define OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU INTERNAL_FUNCTION_PARAM_PASSTHRU, check_type

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

#define SAFE_STRING(a) ((a) ? a : "")
#define BACKSLASH_IN_CLASS _0_

namespace PriorityType
{
enum HookPriority
{
    pZero = 0,
    pFirst = 1,
    pNormal = 2,
    pTotal = 3
};
}
static const int MYSQLI_STORE_RESULT = 0;
static const int MYSQLI_USE_RESULT = 1;
static const int MYSQL_PORT = 3306;

enum OpenRASPActionType
{
    AC_IGNORE = 1,
    AC_LOG = 2,
    AC_BLOCK = 3
};

enum PathOperation
{
    OPENDIR = 1 << 0,
    RENAMESRC = 1 << 1,
    RENAMEDEST = 1 << 2,
    READING = 1 << 3,
    WRITING = 1 << 4,
    APPENDING = 1 << 5,
    SIMULTANEOUSRW = 1 << 6,
    UNLINK = 1 << 7
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
#define DEFINE_HOOK_HANDLER_EX(name, scope, type)                                                                         \
    void scope##_##name##_##type##_handler()                                                                              \
    {                                                                                                                     \
        HashTable *ht = nullptr;                                                                                          \
        zend_function *function;                                                                                          \
        if (strcmp("global", ZEND_TOSTR(scope)) == 0)                                                                     \
        {                                                                                                                 \
            ht = CG(function_table);                                                                                      \
        }                                                                                                                 \
        else                                                                                                              \
        {                                                                                                                 \
            zend_class_entry *clazz;                                                                                      \
            std::string scope_str(ZEND_TOSTR(scope));                                                                     \
            openrasp::string_replace(scope_str, ZEND_TOSTR(BACKSLASH_IN_CLASS), "\\");                                    \
            if ((clazz = static_cast<zend_class_entry *>(                                                                 \
                     zend_hash_str_find_ptr(CG(class_table), scope_str.c_str(), scope_str.length()))) != NULL)            \
            {                                                                                                             \
                ht = &(clazz->function_table);                                                                            \
            }                                                                                                             \
        }                                                                                                                 \
        if (ht &&                                                                                                         \
            (function = static_cast<zend_function *>(zend_hash_str_find_ptr(ht, ZEND_STRL(ZEND_TOSTR(name))))) != NULL && \
            function->internal_function.handler != zif_display_disabled_function)                                         \
        {                                                                                                                 \
            origin_##scope##_##name##_##type = function->internal_function.handler;                                       \
            function->internal_function.handler = hook_##scope##_##name##_##type;                                         \
        }                                                                                                                 \
    }

#define OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                                          \
    php_function origin_##scope##_##name##_##type = nullptr;                                                     \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function); \
    void hook_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAMETERS)                                            \
    {                                                                                                            \
        hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, origin_##scope##_##name##_##type); \
    }                                                                                                            \
    DEFINE_HOOK_HANDLER_EX(name, scope, type);                                                                   \
    int scope##_##name##_##type = []() {register_hook_handler(scope##_##name##_##type##_handler, type, priority);return 0; }();                                                                    \
    inline void hook_##scope##_##name##_##type##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function)

#define OPENRASP_HOOK_FUNCTION_EX(name, scope, type) \
    OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, PriorityType::pNormal)

#define OPENRASP_HOOK_FUNCTION_PRIORITY(name, type, priority) \
    OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, global, type, priority)

#define OPENRASP_HOOK_FUNCTION(name, type) \
    OPENRASP_HOOK_FUNCTION_PRIORITY(name, type, PriorityType::pNormal)

#define HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                        \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);        \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                   \
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

#define HOOK_FUNCTION_EX(name, scope, type) \
    HOOK_FUNCTION_PRIORITY_EX(name, scope, type, PriorityType::pNormal)

#define HOOK_FUNCTION_PRIORITY(name, type, priority) \
    HOOK_FUNCTION_PRIORITY_EX(name, global, type, priority)

#define HOOK_FUNCTION(name, type) \
    HOOK_FUNCTION_PRIORITY(name, type, PriorityType::pNormal)

#define PRE_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                   \
    void pre_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                  \
    {                                                                                \
        bool type_ignored = openrasp_check_type_ignored(type);                       \
        if (!type_ignored)                                                           \
        {                                                                            \
            pre_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
        }                                                                            \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                           \
    }

#define PRE_HOOK_FUNCTION_EX(name, scope, type) \
    PRE_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, PriorityType::pNormal)

#define PRE_HOOK_FUNCTION_PRIORITY(name, type, priority) \
    PRE_HOOK_FUNCTION_PRIORITY_EX(name, global, type, priority)

#define PRE_HOOK_FUNCTION(name, type) \
    PRE_HOOK_FUNCTION_PRIORITY(name, type, PriorityType::pNormal)

#define POST_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                   \
    void post_##scope##_##name##_##type(OPENRASP_INTERNAL_FUNCTION_PARAMETERS);       \
    OPENRASP_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, priority)                   \
    {                                                                                 \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                            \
        bool type_ignored = openrasp_check_type_ignored(type);                        \
        if (!type_ignored)                                                            \
        {                                                                             \
            post_##scope##_##name##_##type(INTERNAL_FUNCTION_PARAM_PASSTHRU, (type)); \
        }                                                                             \
    }

#define POST_HOOK_FUNCTION_EX(name, scope, type) \
    POST_HOOK_FUNCTION_PRIORITY_EX(name, scope, type, PriorityType::pNormal)

#define POST_HOOK_FUNCTION_PRIORITY(name, type, priority) \
    POST_HOOK_FUNCTION_PRIORITY_EX(name, global, type, priority)

#define POST_HOOK_FUNCTION(name, type) \
    POST_HOOK_FUNCTION_PRIORITY(name, type, PriorityType::pNormal)

ZEND_BEGIN_MODULE_GLOBALS(openrasp_hook)
openrasp::dat_value check_type_white_bit_mask;
openrasp::LRU<std::string, bool> lru;
long origin_pg_error_verbos;
std::unordered_set<std::string> callable_blacklist;
std::string echo_filter_regex;
std::unordered_map<uintptr_t, const openrasp::request::ZendRefItem> zend_ref_items;
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

std::string openrasp_real_path(const char *filename, int length, bool use_include_path, uint32_t w_op);

void register_hook_handler(hook_handler_t hook_handler, OpenRASPCheckType type, PriorityType::HookPriority hp = PriorityType::pNormal);

bool openrasp_zval_in_request(zval *item);
bool fetch_name_in_request(zval *item, std::string &name, std::string &type);
bool openrasp_check_type_ignored(OpenRASPCheckType check_type);

void block_handle();
void reset_response();

OpenRASPActionType string_to_action(std::string action_string);
void plugin_ssrf_check(zval *file, const std::string &funtion_name);

#endif