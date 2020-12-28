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

#include "hook/data/copy_object.h"
#include "hook/data/rename_object.h"
#include "hook/data/file_op_object.h"
#include "hook/data/file_put_webshell_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/checker/builtin_detector.h"
#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "utils/string.h"
#include "agent/shared_config_manager.h"

/**
 * 文件相关hook点
 */
PRE_HOOK_FUNCTION(file, READ_FILE);
PRE_HOOK_FUNCTION(file, SSRF);
PRE_HOOK_FUNCTION(readfile, READ_FILE);
PRE_HOOK_FUNCTION(readfile, SSRF);
PRE_HOOK_FUNCTION(file_get_contents, READ_FILE);
PRE_HOOK_FUNCTION(file_get_contents, SSRF);
PRE_HOOK_FUNCTION(file_put_contents, WRITE_FILE);
PRE_HOOK_FUNCTION(file_put_contents, WEBSHELL_FILE_PUT_CONTENTS);
PRE_HOOK_FUNCTION(copy, COPY);
PRE_HOOK_FUNCTION(copy, SSRF);
PRE_HOOK_FUNCTION(rename, RENAME);
PRE_HOOK_FUNCTION(unlink, DELETE_FILE);

using openrasp::end_with;

extern "C" int php_stream_parse_fopen_modes(const char *mode, int *open_flags);

static void pre_global_fopen_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS);
static void pre_splfileobject___construct_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS);
//ref: http://pubs.opengroup.org/onlinepubs/7908799/xsh/open.html
static OpenRASPCheckType flag_to_type(const char *mode);
static void check_file_operation(OpenRASPCheckType type, zval *file, bool use_include_path);

OPENRASP_HOOK_FUNCTION(fopen, READ_FILE)
{
    pre_global_fopen_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

OPENRASP_HOOK_FUNCTION_EX(__construct, splfileobject, READ_FILE)
{
    pre_splfileobject___construct_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

OpenRASPCheckType flag_to_type(const char *mode)
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
    else
    {
        return WRITE_FILE;
    }
}

void check_file_operation(OpenRASPCheckType type, zval *file, bool use_include_path)
{
    openrasp::data::FileOpObject file_obj(file, (type == WRITE_FILE ? WRITING : READING), use_include_path);
    openrasp::checker::V8Detector v8_detector(file_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_file_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_long flags;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "z|l", &filename, &flags) != SUCCESS)
    {
        return;
    }
    if (!maybe_ssrf_vulnerability(filename))
    {
        check_file_operation(check_type, filename, flags & PHP_FILE_USE_INCLUDE_PATH);
    }
}

void pre_global_file_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_long flags;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "z|l", &filename, &flags) != SUCCESS)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(filename))
    {
        plugin_ssrf_check(filename, "file");
    }
}

void pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_bool use_include_path;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "z|b", &filename, &use_include_path) != SUCCESS)
    {
        return;
    }
    if (!maybe_ssrf_vulnerability(filename))
    {
        check_file_operation(check_type, filename, use_include_path);
    }
}

void pre_global_readfile_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_bool use_include_path;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "z|b", &filename, &use_include_path) != SUCCESS)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(filename))
    {
        plugin_ssrf_check(filename, "readfile");
    }
}

void pre_global_file_get_contents_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_file_get_contents_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_readfile_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_file_put_contents_WEBSHELL_FILE_PUT_CONTENTS(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zval *data = nullptr;
    zend_long flags;

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "zz|l", &filename, &data, &flags) != SUCCESS)
    {
        return;
    }

    openrasp::data::FilePutWebshellObject file_webshell_obj(filename, data, flags & PHP_FILE_USE_INCLUDE_PATH);
    openrasp::checker::BuiltinDetector builtin_detector(file_webshell_obj);
    builtin_detector.run();
}

void pre_global_file_put_contents_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zval *data = nullptr;
    zend_long flags;

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "zz|l", &filename, &data, &flags) != SUCCESS)
    {
        return;
    }

    check_file_operation(check_type, filename, (flags & PHP_FILE_USE_INCLUDE_PATH));
}

void pre_global_fopen_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_string *mode = nullptr;
    zend_bool use_include_path;
    mode = nullptr;

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "z|Sb", &filename, &mode, &use_include_path) != SUCCESS)
    {
        return;
    }

    if (maybe_ssrf_vulnerability(filename))
    {
        plugin_ssrf_check(filename, "fopen");
    }
    else
    {
        OpenRASPCheckType type = flag_to_type(mode ? ZSTR_VAL(mode) : "r");
        if (!openrasp_check_type_ignored(type))
        {
            check_file_operation(type, filename, use_include_path);
        }
    }
}

void pre_splfileobject___construct_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zend_string *mode = nullptr;
    zend_bool use_include_path;
    mode = nullptr;

    if (zend_parse_parameters(MIN(3, ZEND_NUM_ARGS()), "z|Sb", &filename, &mode, &use_include_path) != SUCCESS)
    {
        return;
    }

    if (maybe_ssrf_vulnerability(filename))
    {
        plugin_ssrf_check(filename, "splfileobject::__construct");
    }
    else
    {
        OpenRASPCheckType type = flag_to_type(mode ? ZSTR_VAL(mode) : "r");
        if (!openrasp_check_type_ignored(type))
        {
            check_file_operation(type, filename, use_include_path);
        }
    }
}

void pre_global_copy_COPY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *dest = nullptr;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "zz", &source, &dest) != SUCCESS)
    {
        return;
    }
    openrasp::data::CopyObject copy_obj(source, dest);
    openrasp::checker::V8Detector v8_detector(copy_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_copy_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *dest = nullptr;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "zz", &source, &dest) != SUCCESS)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(source))
    {
        plugin_ssrf_check(source, "copy");
    }
}

void pre_global_rename_RENAME(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *dest = nullptr;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "zz", &source, &dest) != SUCCESS)
    {
        return;
    }

    openrasp::data::RenameObject rename_obj(source, dest, OPENRASP_CONFIG(plugin.filter));
    openrasp::checker::V8Detector v8_detector(rename_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_unlink_DELETE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filename = nullptr;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|r", &filename, &zcontext) == FAILURE)
    {
        return;
    }
    openrasp::data::FileOpObject file_obj(filename, UNLINK);
    openrasp::checker::V8Detector v8_detector(file_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}