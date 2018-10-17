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
#include "openrasp_v8.h"

/**
 * directory相关hook点
 */
PRE_HOOK_FUNCTION(dir, DIRECTORY);
PRE_HOOK_FUNCTION(opendir, DIRECTORY);
PRE_HOOK_FUNCTION(scandir, DIRECTORY);

static inline void hook_directory(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (argc > 0 &&
        zend_get_parameters_ex(argc, &path) == SUCCESS &&
        Z_TYPE_PP(path) == IS_STRING)
    {
        char *resolved_path_buff = openrasp_real_path(Z_STRVAL_PP(path), Z_STRLEN_PP(path), false, OPENDIR TSRMLS_CC);
        if (resolved_path_buff)
        {
#if PHP_API_VERSION < 20100412
            if (PG(safe_mode) && (!php_checkuid(resolved_path_buff, NULL, CHECKUID_CHECK_FILE_AND_DIR)))
            {
                return;
            }
#endif
            if (php_check_open_basedir(resolved_path_buff TSRMLS_CC))
            {
                return;
            }

#ifdef ZTS
            if (VCWD_ACCESS(resolved_path_buff, F_OK))
            {
                return;
            }
#endif
            openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
            if (!isolate)
            {
                efree(resolved_path_buff);
                return;
            }
            bool is_block = false;
            {
                v8::HandleScope handle_scope(isolate);
                auto arr = format_debug_backtrace_arr(TSRMLS_C);
                size_t len = arr.size();
                auto stack = v8::Array::New(isolate, len);
                for (size_t i = 0; i < len; i++)
                {
                    stack->Set(i, openrasp::NewV8String(isolate, arr[i]));
                }
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, Z_STRVAL_PP(path), Z_STRLEN_PP(path)));
                params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, resolved_path_buff));
                efree(resolved_path_buff);
                params->Set(openrasp::NewV8String(isolate, "stack"), stack);
                is_block = openrasp::openrasp_check(isolate, openrasp::NewV8String(isolate, CheckTypeNameMap.at(check_type)), params TSRMLS_CC);
            }
            if (is_block)
            {
                handle_block(TSRMLS_C);
            }
        }
    }
}

void pre_global_dir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_opendir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_scandir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    hook_directory(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}