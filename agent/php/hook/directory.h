#pragma once

#include "openrasp_hook.h"

inline void hook_directory(INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (!openrasp_check_type_ignored(ZEND_STRL("directory") TSRMLS_CC) &&
        argc > 0 &&
        zend_get_parameters_ex(argc, &path) == SUCCESS &&
        Z_TYPE_PP(path) == IS_STRING)
    {
        char resolved_path_buff[MAXPATHLEN];
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        add_assoc_zval(params, "path", *path);
        Z_ADDREF_PP(path);
        char *real_path = VCWD_REALPATH(Z_STRVAL_PP(path), resolved_path_buff);
        if (real_path)
        {
            add_assoc_string(params, "realpath", real_path, 1);
        }
        else
        {
            add_assoc_zval(params, "realpath", *path);
            Z_ADDREF_PP(path);
        }
        check("directory", params TSRMLS_CC);
    }
}
#define HOOK_DIRECTORY(name)                               \
    OPENRASP_HOOK_FUNCTION(name)                           \
    {                                                      \
        hook_directory(INTERNAL_FUNCTION_PARAM_PASSTHRU);  \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
    }
HOOK_DIRECTORY(dir);
HOOK_DIRECTORY(opendir);
HOOK_DIRECTORY(scandir);