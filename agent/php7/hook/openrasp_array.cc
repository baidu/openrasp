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
 * callable相关hook点
 */
PRE_HOOK_FUNCTION(array_walk, callable);
PRE_HOOK_FUNCTION(array_map, callable);
PRE_HOOK_FUNCTION(array_filter, callable);

PRE_HOOK_FUNCTION_EX(__construct, reflectionfunction, callable);

static void _callable_handler(const char *functionname, uint functionname_len, const char *check_type TSRMLS_DC)
{
	if (openrasp_check_callable_black(functionname, functionname_len TSRMLS_CC))
	{
		zval attack_params;
		ZVAL_STRING(&attack_params, functionname);
		zval plugin_message;
		ZVAL_STR(&plugin_message, strpprintf(0, _("Webshell detected: using '%s' function"), functionname));
		openrasp_buildin_php_risk_handle(1, check_type, 100, &attack_params, &plugin_message TSRMLS_CC);
	}
}

static void check_callable_function(zend_fcall_info fci, const char *check_type TSRMLS_DC)
{
	if (!ZEND_FCI_INITIALIZED(fci))
	{
		return;
	}
	zval function_name = fci.function_name;
	if (Z_TYPE(function_name) == IS_STRING && Z_STRLEN(function_name) > 0)
	{
		_callable_handler(Z_STRVAL(function_name), Z_STRLEN(function_name), check_type TSRMLS_CC);
	}
}

void pre_global_array_filter_callable(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	if (ZEND_NUM_ARGS() > 1)
	{
		zval *array;
		zend_long use_type = 0;
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "a|fl", &array, &fci, &fci_cache, &use_type) == FAILURE) {
			return;
		}
		check_callable_function(fci, check_type TSRMLS_CC);
	}
}

void pre_global_array_map_callable(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *arrays = NULL;
	int n_arrays = 0;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	// ZEND_PARSE_PARAMETERS_START(2, -1)
	// 	Z_PARAM_FUNC_EX(fci, fci_cache, 1, 0)
	// 	Z_PARAM_VARIADIC('+', arrays, n_arrays)
	// ZEND_PARSE_PARAMETERS_END();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!+", &fci, &fci_cache, &arrays, &n_arrays) == FAILURE) {
		return;
	}

	check_callable_function(fci, check_type TSRMLS_CC);
}

void pre_global_array_walk_callable(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	HashTable *array;
	zval *userdata = NULL;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	// zend_fcall_info orig_array_walk_fci;
	// zend_fcall_info_cache orig_array_walk_fci_cache;

	// orig_array_walk_fci = BG(array_walk_fci);
	// orig_array_walk_fci_cache = BG(array_walk_fci_cache);

	// ZEND_PARSE_PARAMETERS_START(2, 3)
	// 	Z_PARAM_ARRAY_OR_OBJECT_HT_EX(array, 0, 1)
	// 	Z_PARAM_FUNC(BG(array_walk_fci), BG(array_walk_fci_cache))
	// 	Z_PARAM_OPTIONAL
	// 	Z_PARAM_ZVAL_EX(userdata, 0, 1)
	// ZEND_PARSE_PARAMETERS_END_EX(
	// 	BG(array_walk_fci) = orig_array_walk_fci;
	// 	BG(array_walk_fci_cache) = orig_array_walk_fci_cache;
	// 	return
	// );

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hf|z/", &array, &fci, &fci_cache, &userdata) == FAILURE) {
		return;
	}

	check_callable_function(fci, check_type TSRMLS_CC);
}

void pre_reflectionfunction___construct_callable(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *closure = NULL;
	char *lcname, *nsname;
	zend_function *fptr;
	char *name_str;
	size_t name_len;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "o", &closure) == SUCCESS) {
		return;
	} else { 
		if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &name_str, &name_len) == FAILURE) {
			return;
		}

		lcname = zend_str_tolower_dup(name_str, name_len);

		/* Ignore leading "\" */
		nsname = lcname;
		if (lcname[0] == '\\') {
			nsname = &lcname[1];
			name_len--;
		}

		if ((fptr = static_cast<zend_function*>(zend_hash_str_find_ptr(EG(function_table), nsname, name_len))) == NULL) {
			efree(lcname);
			return;
		}
		_callable_handler(nsname, name_len,check_type TSRMLS_CC);
		efree(lcname);
	}
}