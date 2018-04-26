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

#include "openrasp_hook.h"

static inline void hook_directory(INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (!openrasp_check_type_ignored(ZEND_STRL("directory") TSRMLS_CC) &&
        argc > 0 &&
        zend_get_parameters_ex(argc, &path) == SUCCESS &&
        Z_TYPE_PP(path) == IS_STRING)
    {
        char resolved_path_buff[MAXPATHLEN];
        if (VCWD_REALPATH(Z_STRVAL_PP(path), resolved_path_buff))
        {
#if PHP_API_VERSION < 20100412
            if (PG(safe_mode) && (!php_checkuid(resolved_path_buff, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
                return;
            }
#endif

            if (php_check_open_basedir(resolved_path_buff TSRMLS_CC)) {
                return;
            }

#ifdef ZTS
            if (VCWD_ACCESS(resolved_path_buff, F_OK)) {
                return;
            }
#endif
            zval *params;
            MAKE_STD_ZVAL(params);
            array_init(params);
            add_assoc_zval(params, "path", *path);
            Z_ADDREF_PP(path);
            add_assoc_string(params, "realpath", resolved_path_buff, 1);
            zval *stack = NULL;
            MAKE_STD_ZVAL(stack);
            array_init(stack);
            format_debug_backtrace_arr(stack TSRMLS_CC);
            add_assoc_zval(params, "stack", stack);
            check("directory", params TSRMLS_CC);
        }
    }
}

void pre_global_dir(INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_opendir(INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_scandir(INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}