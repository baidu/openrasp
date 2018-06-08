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

/**
 * 文件相关hook点
 */
PRE_HOOK_FUNCTION(file, readFile);
PRE_HOOK_FUNCTION(readfile, readFile);
PRE_HOOK_FUNCTION(file_get_contents, readFile);
PRE_HOOK_FUNCTION(file_put_contents, webshell_file_put_contents);
PRE_HOOK_FUNCTION(file_put_contents, writeFile);
PRE_HOOK_FUNCTION(fopen, readFile);
PRE_HOOK_FUNCTION(fopen, writeFile);
PRE_HOOK_FUNCTION(copy, copy);
PRE_HOOK_FUNCTION(rename, rename);

PRE_HOOK_FUNCTION_EX(__construct, splfileobject, readFile);
PRE_HOOK_FUNCTION_EX(__construct, splfileobject, writeFile);

extern "C" int php_stream_parse_fopen_modes(const char *mode, int *open_flags);

//ref: http://pubs.opengroup.org/onlinepubs/7908799/xsh/open.html
static const char *flag_to_type(int open_flags, bool file_exist)
{
    if (open_flags == O_RDONLY)
    {
        return "readFile";
    }
    else if ((open_flags | O_CREAT) && (open_flags | O_EXCL) && !file_exist)
    {
        return "skip";
    }
    else
    {
        return "writeFile";
    }
}

static void check_file_operation(const char *check_type, char *filename, int filename_len, bool use_include_path TSRMLS_DC)
{
    zend_string *real_path = openrasp_real_path(filename, filename_len, use_include_path, (0 == strcmp(check_type, "writeFile") ? true : false) TSRMLS_CC);
    if (real_path)
    {
        zval params;
        array_init(&params);
        add_assoc_stringl(&params, "path", filename, filename_len);
        add_assoc_str(&params, "realpath", real_path);
        check(check_type, &params TSRMLS_CC);
    }
}

void pre_global_file_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    long flags = 0;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lr!", &filename, &filename_len, &flags, &zcontext) == FAILURE)
    {
        return;
    }
    check_file_operation(check_type, filename, filename_len, flags & PHP_FILE_USE_INCLUDE_PATH TSRMLS_CC);
}

void pre_global_readfile_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
    int filename_len;
    zend_bool use_include_path = 0;
    zval *zcontext = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!", &filename, &filename_len, &use_include_path, &zcontext) == FAILURE)
    {
        return;
    }
    check_file_operation(check_type, filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_file_get_contents_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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
    check_file_operation(check_type, filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_file_put_contents_webshell_file_put_contents(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *path, *data, *flags;
    int argc = MIN(3, ZEND_NUM_ARGS());
    if (argc > 1 &&
        zend_get_parameters_ex(argc, &path, &data, &flags) == SUCCESS &&
        Z_TYPE_P(path) == IS_STRING &&
        openrasp_zval_in_request(path TSRMLS_CC) &&
        openrasp_zval_in_request(data TSRMLS_CC))
    {
        zend_string *real_path = openrasp_real_path(Z_STRVAL_P(path), Z_STRLEN_P(path),
                                                    (argc == 3 && Z_TYPE_P(flags) == IS_LONG && (Z_LVAL_P(flags) & PHP_FILE_USE_INCLUDE_PATH)), true TSRMLS_CC);
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "name", path);
        Z_ADDREF_P(path);
        add_assoc_str(&attack_params, "realpath", real_path);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("Webshell detected - File dropper backdoor"));
        openrasp_buildin_php_risk_handle(1, "webshell_file_put_contents", 100, &attack_params, &plugin_message TSRMLS_CC);
    }
}

void pre_global_file_put_contents_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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

    check_file_operation(check_type, filename, filename_len, (flags & PHP_FILE_USE_INCLUDE_PATH) TSRMLS_CC);
}

void pre_global_fopen_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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
    zend_string *real_path = openrasp_real_path(filename, filename_len, use_include_path, false TSRMLS_CC);
    if (real_path)
    {
        file_exist = true;
        efree(real_path);
    }
    const char *type = flag_to_type(open_flags, file_exist);
    if (0 == strcmp(type, check_type))
    {
        check_file_operation(check_type, filename, filename_len, use_include_path TSRMLS_CC);
    }
}

void pre_global_fopen_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_fopen_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_splfileobject___construct_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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
    zend_string *real_path = openrasp_real_path(filename, filename_len, use_include_path, false TSRMLS_CC);
    if (real_path)
    {
        file_exist = true;
        zend_string_release(real_path);
    }
    const char *type = flag_to_type(open_flags, file_exist);
    if (0 == strcmp(type, check_type))
    {
        check_file_operation(type, filename, filename_len, use_include_path TSRMLS_CC);
    }
}

void pre_splfileobject___construct_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_splfileobject___construct_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_copy_copy(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *source, *target;
    int argc = MIN(2, ZEND_NUM_ARGS());

    if (argc < 2 ||
        zend_get_parameters_ex(argc, &source, &target) != SUCCESS ||
        Z_TYPE_P(source) != IS_STRING ||
        Z_TYPE_P(target) != IS_STRING)
    {
        return;
    }

    zend_string *source_real_path = openrasp_real_path(Z_STRVAL_P(source), Z_STRLEN_P(source), false, false TSRMLS_CC);
    if (source_real_path)
    {
        zend_string *target_real_path = openrasp_real_path(Z_STRVAL_P(target), Z_STRLEN_P(target), false, true TSRMLS_CC);
        zval params;
        array_init(&params);
        add_assoc_str(&params, "source", source_real_path);
        add_assoc_str(&params, "dest", target_real_path);
        check(check_type, &params TSRMLS_CC);
    }
}

void pre_global_rename_rename(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    return pre_global_copy_copy(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}