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
PRE_HOOK_FUNCTION(file_put_contents, WEBSHELL_FILE_PUT_CONTENTS);
PRE_HOOK_FUNCTION(fopen, READ_FILE);
PRE_HOOK_FUNCTION(fopen, WRITE_FILE);
PRE_HOOK_FUNCTION(copy, COPY);
PRE_HOOK_FUNCTION(rename, RENAME);

PRE_HOOK_FUNCTION_EX(__construct, splfileobject, READ_FILE);
PRE_HOOK_FUNCTION_EX(__construct, splfileobject, WRITE_FILE);

using openrasp::end_with;

extern "C" int php_stream_parse_fopen_modes(const char *mode, int *open_flags);

//ref: http://pubs.opengroup.org/onlinepubs/7908799/xsh/open.html
static OpenRASPCheckType flag_to_type(const char *mode, bool is_file_exist)
{
    int open_flags = 0;
    if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
    {
        return INVALID_TYPE;
    }
    if (open_flags == O_RDONLY)
    {
        return READ_FILE;
    }
    else if ((open_flags | O_CREAT) && (open_flags | O_EXCL) && !is_file_exist)
    {
        return INVALID_TYPE;
    }
    else
    {
        return WRITE_FILE;
    }
}

static void check_file_operation(OpenRASPCheckType type, char *filename, int filename_len, bool use_include_path)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
    {
        return;
    }
    std::string real_path = openrasp_real_path(filename, filename_len, use_include_path, (type == WRITE_FILE ? WRITING : READING));
    if (real_path.empty())
    {
        return;
    }
    std::string cache_key = std::string(get_check_type_name(type))
                                .append(filename, filename_len)
                                .append(real_path);
    if (OPENRASP_HOOK_G(lru).contains(cache_key))
    {
        return;
    }
    openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
    {
        v8::HandleScope handle_scope(isolate);
        auto params = v8::Object::New(isolate);
        params->Set(openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, filename, filename_len));
        params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path));
        if (type == WRITE_FILE)
        {
            auto arr = format_debug_backtrace_arr();
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
        handle_block();
    }
}

void pre_global_file_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *filename;
    zend_long flags;

    // ZEND_PARSE_PARAMETERS_START(1, 2)
    // Z_PARAM_STR(filename)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_LONG(flags)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "P|l", &filename, &flags) != SUCCESS)
    {
        return;
    }

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), flags & PHP_FILE_USE_INCLUDE_PATH);
}

void pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *filename;
    zend_bool use_include_path;

    // ZEND_PARSE_PARAMETERS_START(1, 2)
    // Z_PARAM_STR(filename)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_BOOL(use_include_path)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "P|b", &filename, &use_include_path) != SUCCESS)
    {
        return;
    }

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path);
}

void pre_global_file_get_contents_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_file_put_contents_WEBSHELL_FILE_PUT_CONTENTS(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename, *data;
    zend_long flags;

    // ZEND_PARSE_PARAMETERS_START(2, 3)
    // Z_PARAM_ZVAL(filename)
    // Z_PARAM_ZVAL(data)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_LONG(flags)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "zz|l", &filename, &data, &flags) != SUCCESS)
    {
        return;
    }

    if (openrasp_zval_in_request(filename) &&
        openrasp_zval_in_request(data))
    {
        std::string real_path = openrasp_real_path(Z_STRVAL_P(filename), Z_STRLEN_P(filename), flags & PHP_FILE_USE_INCLUDE_PATH, WRITING);
        if (!real_path.empty())
        {
            zval attack_params;
            array_init(&attack_params);
            add_assoc_zval(&attack_params, "name", filename);
            Z_ADDREF_P(filename);
            add_assoc_string(&attack_params, "realpath", const_cast<char*>(real_path.c_str()));
            zval plugin_message;
            ZVAL_STRING(&plugin_message, _("Webshell detected - File dropper backdoor"));
            OpenRASPActionType action = openrasp::scm->get_buildin_check_action(check_type);
            openrasp_buildin_php_risk_handle(action, check_type, 100, &attack_params, &plugin_message);
        }
    }
}

void pre_global_file_put_contents_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *filename;
    zval *data;
    zend_long flags;

    // ZEND_PARSE_PARAMETERS_START(2, 3)
    // Z_PARAM_STR(filename)
    // Z_PARAM_ZVAL(data)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_LONG(flags)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "Pz|l", &filename, &data, &flags) != SUCCESS)
    {
        return;
    }

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), (flags & PHP_FILE_USE_INCLUDE_PATH));
}
static inline void fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *filename, *mode;
    zend_bool use_include_path;
    mode = nullptr;

    // ZEND_PARSE_PARAMETERS_START(2, 3)
    // Z_PARAM_STR(filename)
    // Z_PARAM_STR(mode)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_BOOL(use_include_path)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "P|Sb", &filename, &mode, &use_include_path) != SUCCESS)
    {
        return;
    }

    std::string real_path = openrasp_real_path(ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path, READING);
    OpenRASPCheckType type = flag_to_type(mode ? ZSTR_VAL(mode) : "r", !real_path.empty());
    if (type == check_type)
    {
        check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path);
    }
}
void pre_global_fopen_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (ZEND_NUM_ARGS() >= 2)
    {
        fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

void pre_global_fopen_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (ZEND_NUM_ARGS() >= 2)
    {
        fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

void pre_splfileobject___construct_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_splfileobject___construct_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_copy_COPY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
    {
        return;
    }
    zend_string *source, *dest;

    // ZEND_PARSE_PARAMETERS_START(2, 2)
    // Z_PARAM_STR(source)
    // Z_PARAM_STR(dest)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "PP", &source, &dest) != SUCCESS)
    {
        return;
    }

    std::string source_real_path = openrasp_real_path(ZSTR_VAL(source), ZSTR_LEN(source), false, READING);
    if (source_real_path.empty())
    {
        return;
    }
    std::string dest_real_path = openrasp_real_path(ZSTR_VAL(dest), ZSTR_LEN(dest), false, WRITING);
    if (dest_real_path.empty())
    {
        return;
    }
    std::string cache_key = std::string(get_check_type_name(check_type))
                                .append(source_real_path)
                                .append(dest_real_path);
    if (OPENRASP_HOOK_G(lru).contains(cache_key))
    {
        return;
    }
    openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
    {
        v8::HandleScope handle_scope(isolate);
        auto params = v8::Object::New(isolate);
        params->Set(openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_real_path));
        params->Set(openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, dest_real_path));
        check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
    }
    if (check_result == openrasp::CheckResult::kCache)
    {
        OPENRASP_HOOK_G(lru).set(cache_key, true);
    }
    if (check_result == openrasp::CheckResult::kBlock)
    {
        handle_block();
    }
}

void pre_global_rename_RENAME(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
    {
        return;
    }
    zend_string *source, *dest;

    // ZEND_PARSE_PARAMETERS_START(2, 2)
    // Z_PARAM_STR(source)
    // Z_PARAM_STR(dest)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "PP", &source, &dest) != SUCCESS)
    {
        return;
    }

    std::string source_real_path = openrasp_real_path(ZSTR_VAL(source), ZSTR_LEN(source), false, RENAMESRC);
    std::string dest_real_path = openrasp_real_path(ZSTR_VAL(dest), ZSTR_LEN(dest), false, RENAMEDEST);
    if (!source_real_path.empty() && !dest_real_path.empty())
    {
        bool skip = false;
        const char *src_scheme = fetch_url_scheme(source_real_path.c_str());
        const char *tgt_scheme = fetch_url_scheme(dest_real_path.c_str());
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
                if (VCWD_STAT(dest_real_path.c_str(), &tgt_sb) == 0)
                {
                    if ((tgt_sb.st_mode & S_IFDIR) != 0)
                    {
                        skip = true;
                    }
                }
                else
                {
                    if (end_with(dest_real_path, std::string(1, DEFAULT_SLASH)))
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
            std::string cache_key = std::string(get_check_type_name(check_type))
                                        .append(source_real_path)
                                        .append(dest_real_path);
            if (OPENRASP_HOOK_G(lru).contains(cache_key))
            {
                return;
            }
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_real_path));
                params->Set(openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, dest_real_path));
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kCache)
            {
                OPENRASP_HOOK_G(lru).set(cache_key, true);
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block();
            }
        }
    }
}