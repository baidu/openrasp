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

static const char* mode_to_type(char *mode)
{
    if (strchr(mode, '+') > mode || strchr(mode, 'w') > mode || strchr(mode, 'a') > mode)
    {
        return "writeFile";
    }
    else //r,rb,rt
    {
        return "readFile";
    }
}

//return value estrdup
char * openrasp_real_path(char *filename, int filename_len, zend_bool use_include_path, bool handle_unresolved TSRMLS_DC)
{
    php_url *resource = php_url_parse_ex(filename, filename_len);
    char *real_path = nullptr;
    if (resource && resource->scheme) 
    {
        real_path = estrdup(filename);
    }
    else
    {
        char expand_path[MAXPATHLEN];
        if (!expand_filepath(filename, expand_path TSRMLS_CC)) {
            return real_path;
        }
        real_path = php_resolve_path(expand_path, strlen(expand_path), use_include_path ? PG(include_path) : NULL TSRMLS_CC);
        if (!real_path && handle_unresolved)
        {
            real_path = estrdup(expand_path);
        }
    }
    if (resource) {
        php_url_free(resource);
    }
    return real_path;
}

static void check_file_operation(const char* type, char *filename, int filename_len, zend_bool use_include_path TSRMLS_DC)
{	
    char *real_path = openrasp_real_path(filename, filename_len, use_include_path, (0 == strcmp(type, "writeFile") ? true : false) TSRMLS_CC);
    if (real_path)
    {
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        add_assoc_string(params, "path", filename, 1);
        add_assoc_string(params, "realpath", real_path, 0);
        check(type, params TSRMLS_CC);
    }
}

void pre_global_file_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
	int filename_len;
	long flags = 0;
    zend_bool use_include_path;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lr!", &filename, &filename_len, &flags, &zcontext) == FAILURE) {
		return;
	}
    use_include_path = flags & PHP_FILE_USE_INCLUDE_PATH;
    check_file_operation(check_type, filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_readfile_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
	int filename_len;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!", &filename, &filename_len, &use_include_path, &zcontext) == FAILURE) {
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!ll", &filename, &filename_len, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE) {
		return;
	}
    check_file_operation(check_type, filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_file_put_contents_webshell_file_put_contents(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path, **data, **flags;
    int argc = MIN(3, ZEND_NUM_ARGS());
    if (argc > 1 
    && zend_get_parameters_ex(argc, &path, &data, &flags) == SUCCESS 
    && Z_TYPE_PP(path) == IS_STRING
    && openrasp_zval_in_request(*path TSRMLS_CC)
    && openrasp_zval_in_request(*data TSRMLS_CC))
    {
        char *real_path = openrasp_real_path(Z_STRVAL_PP(path), Z_STRLEN_PP(path), 
        (argc == 3 && Z_TYPE_PP(flags) == IS_LONG && (Z_LVAL_PP(flags) & PHP_FILE_USE_INCLUDE_PATH)), true TSRMLS_CC);
        zval *attack_params = NULL;
        MAKE_STD_ZVAL(attack_params);
        array_init(attack_params);
        add_assoc_zval(attack_params, "name", *path);
        Z_ADDREF_P(*path);
        add_assoc_string(attack_params, "realpath", real_path, 0);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        ZVAL_STRING(plugin_message, _("Webshell detected - File dropper backdoor"), 1);
        openrasp_buildin_php_risk_handle(1, "webshell_file_put_contents", 100, attack_params, plugin_message TSRMLS_CC);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz/|lr!", &filename, &filename_len, &data, &flags, &zcontext) == FAILURE) {
		return;
	}

	if (strlen(filename) != filename_len) {
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|br", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		return;
	}
    const char *type = mode_to_type(mode);
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sbr", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		return;
	}
    const char *type = mode_to_type(mode);
    if (0 == strcmp(type, check_type))
    {
        check_file_operation(type, filename, filename_len, use_include_path TSRMLS_CC);
    }
}

void pre_splfileobject___construct_readFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_splfileobject___construct_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_copy_writeFile(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *source, *target;
	int source_len, target_len;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|r", &source, &source_len, &target, &target_len, &zcontext) == FAILURE) {
		return;
	}

    if (source && target && strlen(source) == source_len && strlen(target) == target_len) {
        check_file_operation(check_type, target, target_len, 0 TSRMLS_CC);
	}
}