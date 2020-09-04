/*
 * Copyright 2017-2020 Baidu Inc.
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
PRE_HOOK_FUNCTION(file_put_contents, WEBSHELL_FILE_PUT_CONTENTS); //must after PRE_HOOK_FUNCTION(file_put_contents, writeFile);
PRE_HOOK_FUNCTION(copy, COPY);
PRE_HOOK_FUNCTION(copy, SSRF);
PRE_HOOK_FUNCTION(rename, RENAME);
PRE_HOOK_FUNCTION(unlink, DELETE_FILE);

using openrasp::end_with;
extern "C" int php_stream_parse_fopen_modes(const char *mode, int *open_flags);
static void pre_global_fopen_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS);
static void pre_splfileobject___construct_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS);
static OpenRASPCheckType flag_to_type(int open_flags);
static void check_file_operation(OpenRASPCheckType type, zval *file, zend_bool use_include_path TSRMLS_DC);

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

OpenRASPCheckType flag_to_type(int open_flags)
{
    if (open_flags == O_RDONLY)
    {
        return READ_FILE;
    }
    else
    {
        return WRITE_FILE;
    }
}

void check_file_operation(OpenRASPCheckType type, zval *file, zend_bool use_include_path TSRMLS_DC)
{
    openrasp::data::FileOpObject file_obj(file, (type == WRITE_FILE ? WRITING : READING), use_include_path);
    openrasp::checker::V8Detector v8_detector(file_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_file_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    long flags = 0;
    zend_bool use_include_path;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lr!", &file, &flags, &zcontext) == FAILURE)
    {
        return;
    }
    use_include_path = flags & PHP_FILE_USE_INCLUDE_PATH;
    if (!maybe_ssrf_vulnerability(file))
    {
        check_file_operation(check_type, file, use_include_path TSRMLS_CC);
    }
}

void pre_global_file_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    long flags = 0;
    zend_bool use_include_path;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lr!", &file, &flags, &zcontext) == FAILURE)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(file))
    {
        plugin_ssrf_check(file, "file" TSRMLS_CC);
    }
}

void pre_global_readfile_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zend_bool use_include_path = 0;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|br!", &file, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    if (!maybe_ssrf_vulnerability(file))
    {
        check_file_operation(check_type, file, use_include_path TSRMLS_CC);
    }
}

void pre_global_readfile_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zend_bool use_include_path = 0;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|br!", &file, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(file))
    {
        plugin_ssrf_check(file, "readfile" TSRMLS_CC);
    }
}

void pre_global_file_get_contents_READ_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zend_bool use_include_path = 0;
    long offset = -1;
    long maxlen = PHP_STREAM_COPY_ALL;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|br!ll", &file, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE)
    {
        return;
    }
    if (!maybe_ssrf_vulnerability(file))
    {
        check_file_operation(check_type, file, use_include_path TSRMLS_CC);
    }
}

void pre_global_file_get_contents_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zend_bool use_include_path = 0;
    long offset = -1;
    long maxlen = PHP_STREAM_COPY_ALL;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|br!ll", &file, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(file))
    {
        plugin_ssrf_check(file, "file_get_contents" TSRMLS_CC);
    }
}

void pre_global_file_put_contents_WEBSHELL_FILE_PUT_CONTENTS(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path, **data, **flags;
    int argc = MIN(3, ZEND_NUM_ARGS());
    if (argc > 1 && zend_get_parameters_ex(argc, &path, &data, &flags) == SUCCESS && Z_TYPE_PP(path) == IS_STRING && openrasp_zval_in_request(*path) && openrasp_zval_in_request(*data))
    {
        openrasp::data::FilePutWebshellObject file_webshell_obj(*path, *data, (argc == 3 && Z_TYPE_PP(flags) == IS_LONG && (Z_LVAL_PP(flags) & PHP_FILE_USE_INCLUDE_PATH)));
        openrasp::checker::BuiltinDetector builtin_detector(file_webshell_obj);
        builtin_detector.run();
    }
}

void pre_global_file_put_contents_WRITE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zval *data = nullptr;
    int numbytes = 0;
    long flags = 0;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz/|lr!", &file, &data, &flags, &zcontext) == FAILURE)
    {
        return;
    }

    check_file_operation(check_type, file, (flags & PHP_FILE_USE_INCLUDE_PATH) TSRMLS_CC);
}

void pre_global_fopen_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    char *mode = nullptr;
    int mode_len = 0;
    zend_bool use_include_path = 0;
    zval *zcontext = nullptr;
    int open_flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs|br", &file, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(file))
    {
        plugin_ssrf_check(file, "fopen" TSRMLS_CC);
    }
    else
    {
        if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
        {
            return;
        }
        OpenRASPCheckType type = flag_to_type(open_flags);
        if (!openrasp_check_type_ignored(type TSRMLS_CC))
        {
            check_file_operation(type, file, use_include_path TSRMLS_CC);
        }
    }
}

void pre_splfileobject___construct_READ_WRITE_FILE_SSRF(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    char *mode = nullptr;
    int mode_len = 0;
    zend_bool use_include_path = 0;
    zval *zcontext = nullptr;
    int open_flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|sbr", &file, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }

    if (nullptr == mode)
    {
        mode = "r";
        mode_len = 1;
    }

    if (maybe_ssrf_vulnerability(file))
    {
        plugin_ssrf_check(file, "splfileobject::__construct" TSRMLS_CC);
    }
    else
    {
        if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
        {
            return;
        }
        OpenRASPCheckType type = flag_to_type(open_flags);
        if (!openrasp_check_type_ignored(type TSRMLS_CC))
        {
            check_file_operation(type, file, use_include_path TSRMLS_CC);
        }
    }
}

void pre_global_copy_COPY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *target = nullptr;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|r", &source, &target, &zcontext) == FAILURE)
    {
        return;
    }
    openrasp::data::CopyObject copy_obj(source, target);
    openrasp::checker::V8Detector v8_detector(copy_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_copy_SSRF(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *target = nullptr;
    zval *zcontext = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|r", &source, &target, &zcontext) == FAILURE)
    {
        return;
    }
    if (maybe_ssrf_vulnerability(source))
    {
        plugin_ssrf_check(source, "copy" TSRMLS_CC);
    }
}

void pre_global_rename_RENAME(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source = nullptr;
    zval *target = nullptr;
    zval *zcontext = nullptr;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|r", &source, &target, &zcontext) == FAILURE)
    {
        return;
    }
    openrasp::data::RenameObject rename_obj(source, target, OPENRASP_CONFIG(plugin.filter));
    openrasp::checker::V8Detector v8_detector(rename_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

void pre_global_unlink_DELETE_FILE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *file = nullptr;
    zval *zcontext = nullptr;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|r", &file, &zcontext) == FAILURE)
    {
        return;
    }
    openrasp::data::FileOpObject file_obj(file, UNLINK);
    openrasp::checker::V8Detector v8_detector(file_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}