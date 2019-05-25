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

#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "utils/string.h"
#include "agent/shared_config_manager.h"

/**
 * 文件相关hook点
 */
PRE_HOOK_FUNCTION(file, READ_FILE);
PRE_HOOK_FUNCTION(readfile, READ_FILE);
PRE_HOOK_FUNCTION(file_get_contents, READ_FILE);
PRE_HOOK_FUNCTION(file_put_contents, WRITE_FILE);
PRE_HOOK_FUNCTION(file_put_contents, WEBSHELL_FILE_PUT_CONTENTS); //must after PRE_HOOK_FUNCTION(file_put_contents, writeFile);
PRE_HOOK_FUNCTION(fopen, READ_FILE);
PRE_HOOK_FUNCTION(fopen, WRITE_FILE);
PRE_HOOK_FUNCTION(copy, COPY);
PRE_HOOK_FUNCTION(rename, RENAME);

PRE_HOOK_FUNCTION_EX(__construct, splfileobject, READ_FILE);
PRE_HOOK_FUNCTION_EX(__construct, splfileobject, WRITE_FILE);

using openrasp::end_with;
extern "C" int php_stream_parse_fopen_modes(const char *mode, int *open_flags);

//ref: http://pubs.opengroup.org/onlinepubs/7908799/xsh/open.html
static OpenRASPCheckType flag_to_type(int open_flags, bool file_exist)
{
    if (open_flags == O_RDONLY)
    {
        return READ_FILE;
    }
    else if ((open_flags | O_CREAT) && (open_flags | O_EXCL) && !file_exist)
    {
        return INVALID_TYPE;
    }
    else
    {
        return WRITE_FILE;
    }
}

static void check_file_operation(OpenRASPCheckType type, const std::string &filename, zend_bool use_include_path TSRMLS_DC)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (isolate)
    {
        std::string real_path = openrasp_real_path(const_cast<char *>(filename.c_str()), filename.length(), use_include_path, (type == WRITE_FILE ? WRITING : READING) TSRMLS_CC);
        if (!real_path.empty())
        {
            const std::string realpath(real_path);
            const std::string check_type(get_check_type_name(type));
            const std::string cache_key(check_type + filename + realpath);
            if (OPENRASP_HOOK_G(lru).contains(cache_key))
            {
                return;
            }
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, filename));
                params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, realpath));
                if (type == WRITE_FILE)
                {
                    auto arr = format_debug_backtrace_arr(TSRMLS_C);
                    size_t len = arr.size();
                    auto stack = v8::Array::New(isolate, len);
                    for (size_t i = 0; i < len; i++)
                    {
                        stack->Set(i, openrasp::NewV8String(isolate, arr[i]));
                    }
                    params->Set(openrasp::NewV8String(isolate, "stack"), stack);
                }
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kCache)
            {
                OPENRASP_HOOK_G(lru).set(cache_key, true);
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block(TSRMLS_C);
            }
        }
    }
}

void pre_global_file_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    long flags = 0;
    zend_bool use_include_path;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lr!", &filename, &filename_len, &flags, &zcontext) == FAILURE)
    {
        return;
    }
    use_include_path = flags & PHP_FILE_USE_INCLUDE_PATH;
    check_file_operation(check_type, std::string(filename, filename_len), use_include_path TSRMLS_CC);
}

void pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    zend_bool use_include_path = 0;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!", &filename, &filename_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    check_file_operation(check_type, std::string(filename, filename_len), use_include_path TSRMLS_CC);
}

void pre_global_file_get_contents_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    zend_bool use_include_path = 0;
    long offset = -1;
    long maxlen = PHP_STREAM_COPY_ALL;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!ll", &filename, &filename_len, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE)
    {
        return;
    }
    check_file_operation(check_type, std::string(filename, filename_len), use_include_path TSRMLS_CC);
}

void pre_global_file_put_contents_WEBSHELL_FILE_PUT_CONTENTS(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path, **data, **flags;
    int argc = MIN(3, ZEND_NUM_ARGS());
    if (argc > 1 && zend_get_parameters_ex(argc, &path, &data, &flags) == SUCCESS && Z_TYPE_PP(path) == IS_STRING && openrasp_zval_in_request(*path TSRMLS_CC) && openrasp_zval_in_request(*data TSRMLS_CC))
    {
        std::string real_path = openrasp_real_path(Z_STRVAL_PP(path), Z_STRLEN_PP(path),
                                                   (argc == 3 && Z_TYPE_PP(flags) == IS_LONG && (Z_LVAL_PP(flags) & PHP_FILE_USE_INCLUDE_PATH)),
                                                   WRITING TSRMLS_CC);
        if (!real_path.empty())
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_zval(attack_params, "name", *path);
            Z_ADDREF_P(*path);
            add_assoc_string(attack_params, "realpath", const_cast<char *>(real_path.c_str()), 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            ZVAL_STRING(plugin_message, _("WebShell activity - Detected file dropper backdoor"), 1);
            OpenRASPActionType action = openrasp::scm->get_buildin_check_action(check_type);
            openrasp_buildin_php_risk_handle(action, check_type, 100, attack_params, plugin_message TSRMLS_CC);
        }
    }
}

void pre_global_file_put_contents_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    zval *data;
    int numbytes = 0;
    long flags = 0;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz/|lr!", &filename, &filename_len, &data, &flags, &zcontext) == FAILURE)
    {
        return;
    }

    if (strlen(filename) != filename_len)
    {
        return;
    }

    check_file_operation(check_type, std::string(filename, filename_len), (flags & PHP_FILE_USE_INCLUDE_PATH) TSRMLS_CC);
}

void pre_global_fopen_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename, *mode;
    int filename_len, mode_len;
    zend_bool use_include_path = 0;
    zval *zcontext = NULL;
    bool file_exist = false;
    int open_flags;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|br", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
    {
        return;
    }
    std::string real_path = openrasp_real_path(filename, filename_len, use_include_path, READING TSRMLS_CC);
    if (!real_path.empty())
    {
        file_exist = true;
    }
    OpenRASPCheckType type = flag_to_type(open_flags, file_exist);
    if (type == check_type)
    {
        check_file_operation(check_type, std::string(filename, filename_len), use_include_path TSRMLS_CC);
    }
}

void pre_global_fopen_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_fopen_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_splfileobject___construct_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename, *mode;
    int filename_len, mode_len;
    zend_bool use_include_path = 0;
    zval *zcontext = NULL;
    bool file_exist = false;
    int open_flags;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sbr", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
    {
        return;
    }
    std::string real_path = openrasp_real_path(filename, filename_len, use_include_path, READING TSRMLS_CC);
    if (!real_path.empty())
    {
        file_exist = true;
    }
    OpenRASPCheckType type = flag_to_type(open_flags, file_exist);
    if (type == check_type)
    {
        check_file_operation(type, std::string(filename, filename_len), use_include_path TSRMLS_CC);
    }
}

void pre_splfileobject___construct_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_splfileobject___construct_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_copy_COPY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
    {
        return;
    }
    char *source, *target;
    int source_len, target_len;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|r", &source, &source_len, &target, &target_len, &zcontext) == FAILURE)
    {
        return;
    }

    if (source && target && strlen(source) == source_len && strlen(target) == target_len)
    {
        std::string source_real_path = openrasp_real_path(source, source_len, false, READING TSRMLS_CC);
        std::string target_real_path = openrasp_real_path(target, target_len, false, WRITING TSRMLS_CC);
        if (!source_real_path.empty() && !target_real_path.empty())
        {
            const std::string type(get_check_type_name(check_type));
            const std::string cache_key(type + source_real_path + target_real_path);
            if (OPENRASP_HOOK_G(lru).contains(cache_key))
            {
                return;
            }
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_real_path));
                params->Set(openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, target_real_path));
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kCache)
            {
                OPENRASP_HOOK_G(lru).set(cache_key, true);
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block(TSRMLS_C);
            }
        }
    }
}

void pre_global_rename_RENAME(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
    {
        return;
    }
    char *source, *target;
    int source_len, target_len;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|r", &source, &source_len, &target, &target_len, &zcontext) == FAILURE)
    {
        return;
    }

    if (nullptr == source || nullptr == target || strlen(source) != source_len || strlen(target) != target_len)
    {
        return;
    }

    std::string source_real_path = openrasp_real_path(source, source_len, false, RENAMESRC TSRMLS_CC);
    std::string target_real_path = openrasp_real_path(target, target_len, false, RENAMEDEST TSRMLS_CC);
    if (!source_real_path.empty() && !target_real_path.empty())
    {
        bool skip = false;
        const char *src_scheme = fetch_url_scheme(source_real_path.c_str());
        const char *tgt_scheme = fetch_url_scheme(target_real_path.c_str());
        if (src_scheme && tgt_scheme)
        {
            if (strcmp(src_scheme, tgt_scheme) != 0)
            {
                skip = true;
            }
        }
        else if (!src_scheme && !tgt_scheme)
        {
            struct stat src_sb;
            if (VCWD_STAT(source_real_path.c_str(), &src_sb) == 0 && (src_sb.st_mode & S_IFDIR) != 0)
            {
                skip = true;
            }
            else
            {
                struct stat tgt_sb;
                if (VCWD_STAT(target_real_path.c_str(), &tgt_sb) == 0)
                {
                    if ((tgt_sb.st_mode & S_IFDIR) != 0)
                    {
                        skip = true;
                    }
                }
                else
                {
                    if (end_with(target_real_path, std::string(1, DEFAULT_SLASH)))
                    {
                        skip = true;
                    }
                }
            }
        }
        else
        {
            skip = true;
        }

        if (!skip)
        {
            const std::string type(get_check_type_name(check_type));
            const std::string cache_key(type + source_real_path + target_real_path);
            if (OPENRASP_HOOK_G(lru).contains(cache_key))
            {
                return;
            }
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_real_path));
                params->Set(openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, target_real_path));
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kCache)
            {
                OPENRASP_HOOK_G(lru).set(cache_key, true);
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block(TSRMLS_C);
            }
        }
    }
}