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
static const char *flag_to_type(const char *mode, bool is_file_exist)
{
    int open_flags = 0;
    if (FAILURE == php_stream_parse_fopen_modes(mode, &open_flags))
    {
        return "";
    }
    if (open_flags == O_RDONLY)
    {
        return "readFile";
    }
    else if ((open_flags | O_CREAT) && (open_flags | O_EXCL) && !is_file_exist)
    {
        return "";
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

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), flags & PHP_FILE_USE_INCLUDE_PATH TSRMLS_CC);
}

void pre_global_readfile_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path TSRMLS_CC);
}

void pre_global_file_get_contents_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_readfile_readFile(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_file_put_contents_webshell_file_put_contents(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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

    if (openrasp_zval_in_request(filename TSRMLS_CC) &&
        openrasp_zval_in_request(data TSRMLS_CC))
    {
        zend_string *real_path = openrasp_real_path(Z_STRVAL_P(filename), Z_STRLEN_P(filename), flags & PHP_FILE_USE_INCLUDE_PATH, true TSRMLS_CC);
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "name", filename);
        Z_ADDREF_P(filename);
        add_assoc_str(&attack_params, "realpath", real_path);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("Webshell detected - File dropper backdoor"));
        openrasp_buildin_php_risk_handle(1, "webshell_file_put_contents", 100, &attack_params, &plugin_message TSRMLS_CC);
    }
}

void pre_global_file_put_contents_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
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

    check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), (flags & PHP_FILE_USE_INCLUDE_PATH) TSRMLS_CC);
}
static inline void fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *filename, *mode;
    zend_bool use_include_path;

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

    zend_string *real_path = openrasp_real_path(ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path, false TSRMLS_CC);
    const char *type = flag_to_type(mode ? ZSTR_VAL(mode) : "r", real_path);
    if (real_path)
    {
        zend_string_release(real_path);
    }
    if (0 == strcmp(type, check_type))
    {
        check_file_operation(check_type, ZSTR_VAL(filename), ZSTR_LEN(filename), use_include_path TSRMLS_CC);
    }
}
void pre_global_fopen_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (ZEND_NUM_ARGS() >= 2)
    {
        fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

void pre_global_fopen_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (ZEND_NUM_ARGS() >= 2)
    {
        fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

void pre_splfileobject___construct_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_splfileobject___construct_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    fopen_common_handler(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_copy_copy(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *source, *dest;

    // ZEND_PARSE_PARAMETERS_START(2, 2)
    // Z_PARAM_STR(source)
    // Z_PARAM_STR(dest)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "PP", &source, &dest) != SUCCESS)
    {
        return;
    }

    zend_string *source_real_path = openrasp_real_path(ZSTR_VAL(source), ZSTR_LEN(source), false, false TSRMLS_CC);
    if (source_real_path)
    {
        zend_string *dest_real_path = openrasp_real_path(ZSTR_VAL(dest), ZSTR_LEN(dest), false, true TSRMLS_CC);
        zval params;
        array_init(&params);
        add_assoc_str(&params, "source", source_real_path);
        add_assoc_str(&params, "dest", dest_real_path);
        check(check_type, &params TSRMLS_CC);
    }
}

void pre_global_rename_rename(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_copy_copy(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}