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
 * directory相关hook点
 */
PRE_HOOK_FUNCTION(dir, directory);
PRE_HOOK_FUNCTION(opendir, directory);
PRE_HOOK_FUNCTION(scandir, directory);

static void _check_dirname(char *dirname, size_t dir_len, const char *check_type)
{
	if (dir_len < 1)
	{
		return;
	}
	php_stream *dirp;
	dirp = php_stream_opendir(dirname, IGNORE_PATH, NULL);
	if (!dirp)
	{
		return;
	}
	php_stream_close(dirp);
	char resolved_path_buff[MAXPATHLEN];
	if (VCWD_REALPATH(dirname, resolved_path_buff))
	{
		zval params;
		array_init(&params);
		add_assoc_string(&params, "path", dirname);
		add_assoc_string(&params, "realpath", resolved_path_buff);
		zval stack;
		array_init(&stack);
		format_debug_backtrace_arr(&stack TSRMLS_CC);
		add_assoc_zval(&params, "stack", &stack);
		check(check_type, &params TSRMLS_CC);
	}
}

static void _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *dirname;
	size_t dir_len;
	zval *zcontext = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_PATH(dirname, dir_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_RESOURCE(zcontext)
	ZEND_PARSE_PARAMETERS_END();

	_check_dirname(dirname, dir_len, check_type);
}

void pre_global_dir_directory(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_opendir_directory(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_scandir_directory(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	char *dirn;
	size_t dirn_len;
	zend_long flags = 0;
	zval *zcontext = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_PATH(dirn, dirn_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flags)
		Z_PARAM_RESOURCE(zcontext)
	ZEND_PARSE_PARAMETERS_END();

	_check_dirname(dirn, dirn_len, check_type);
}