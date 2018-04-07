#ifndef OPENRASP_HOOK_H
#define OPENRASP_HOOK_H

#include "openrasp.h"
#include "openrasp_log.h"

extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "ext/standard/file.h"
#include "ext/standard/url.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_API.h"
#include "Zend/zend_compile.h"
}
#include <string>
#include <set>

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
#define OPENRASP_HOOK_FUNCTION_EX(name, scope)                                                          \
    php_function origin_##scope##_##name = nullptr;                                                     \
    inline void hook_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function); \
    void hook_##scope##_##name(INTERNAL_FUNCTION_PARAMETERS)                                            \
    {                                                                                                   \
        hook_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, origin_##scope##_##name);          \
    }                                                                                                   \
    inline void hook_##scope##_##name##_ex(INTERNAL_FUNCTION_PARAMETERS, php_function origin_function)
#define OPENRASP_HOOK_FUNCTION(name) \
    OPENRASP_HOOK_FUNCTION_EX(name, global)

#define HOOK_FUNCTION_EX(name, scope)                                                                   \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                                                              \
    {                                                                                                   \
        pre_##scope##_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                         \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                              \
        post_##scope##_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                        \
    }

#define HOOK_FUNCTION(name) \
    HOOK_FUNCTION_EX(name, global)

#define PRE_HOOK_FUNCTION_EX(name, scope)                                                               \
    OPENRASP_HOOK_FUNCTION_EX(name, scope)                                                              \
    {                                                                                                   \
        pre_##scope##_##name(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                         \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);                                              \
    }

#define PRE_HOOK_FUNCTION(name) \
    PRE_HOOK_FUNCTION_EX(name, global)
/**
 * 使用这个宏 hook 指定函数
 * 需要在 hook 前先定义相应的替换函数
 * 这个宏只在 compiler_global 中查找对应的函数或类，所以可以在 MINIT 中使用
 * 
 * @param name 函数完整名称
 * @param scope 函数所属 class，全局函数的 scope 为 global
 */
#define OPENRASP_HOOK_EX(name, scope)                                                                      \
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
            origin_##scope##_##name = function->internal_function.handler;                                 \
            function->internal_function.handler = hook_##scope##_##name;                                   \
        }                                                                                                  \
    }
#define OPENRASP_HOOK(name) \
    OPENRASP_HOOK_EX(name, global)

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

#endif