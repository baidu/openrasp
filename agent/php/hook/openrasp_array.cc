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

static void check_callable_function(zend_fcall_info fci TSRMLS_DC)
{
    if (openrasp_check_type_ignored(ZEND_STRL("callable") TSRMLS_CC))
    {
        return;
    }
	if (!ZEND_FCI_INITIALIZED(fci))
	{
		return;
	}
	zval *function_name = fci.function_name;
	if (Z_TYPE_P(function_name) == IS_STRING && Z_STRLEN_P(function_name) > 0)
	{
		if (openrasp_check_callable_black(Z_STRVAL_P(function_name), Z_STRLEN_P(function_name) TSRMLS_CC))
		{
			zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            ZVAL_STRING(attack_params, Z_STRVAL_P(function_name), 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            char *message_str = NULL;
            spprintf(&message_str, 0, _("Webshell detected: using '%s' function"), Z_STRVAL_P(function_name));
            ZVAL_STRING(plugin_message, message_str, 1);
            efree(message_str);
            openrasp_buildin_php_risk_handle(1, "callable", 100, attack_params, plugin_message TSRMLS_CC);
		}
	}
}

static void pre_php_array_diff(INTERNAL_FUNCTION_PARAMETERS, int behavior, int data_compare_type, int key_compare_type)
{
    zval ***args = NULL;
	int arr_argc;
	int req_args;
	char *param_spec;
	zend_fcall_info fci1, fci2;
	zend_fcall_info_cache fci1_cache = empty_fcall_info_cache, fci2_cache = empty_fcall_info_cache;
	zend_fcall_info *fci_key = NULL, *fci_data;
	zend_fcall_info_cache *fci_key_cache = NULL, *fci_data_cache;

	if (behavior == DIFF_NORMAL) {

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL) {
			/* array_diff */
			req_args = 2;
			param_spec = (char *)"+";
		} else if (data_compare_type == DIFF_COMP_DATA_USER) {
			/* array_udiff */
			req_args = 3;
			param_spec = (char *)"+f";
		} else {
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache) == FAILURE) {
			return;
		}
		fci_data = &fci1;
		fci_data_cache = &fci1_cache;

	} else if (behavior & DIFF_ASSOC) { /* triggered also if DIFF_KEY */
		/* DIFF_KEY is subset of DIFF_ASSOC. When having the former
		 * no comparison of the data is done (part of DIFF_ASSOC) */

		if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_diff_assoc() or array_diff_key() */
			req_args = 2;
			param_spec = (char *)"+";
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_INTERNAL) {
			/* array_udiff_assoc() */
			req_args = 3;
			param_spec = (char *)"+f";
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_INTERNAL && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_diff_uassoc() or array_diff_ukey() */
			req_args = 3;
			param_spec = (char *)"+f";
			fci_key = &fci1;
			fci_key_cache = &fci1_cache;
		} else if (data_compare_type == DIFF_COMP_DATA_USER && key_compare_type == DIFF_COMP_KEY_USER) {
			/* array_udiff_uassoc() */
			req_args = 4;
			param_spec = (char *)"+ff";
			fci_data = &fci1;
			fci_data_cache = &fci1_cache;
			fci_key = &fci2;
			fci_key_cache = &fci2_cache;
		} else {
			return;
		}

		if (ZEND_NUM_ARGS() < req_args) {
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, param_spec, &args, &arr_argc, &fci1, &fci1_cache, &fci2, &fci2_cache) == FAILURE) {
			return;
		}

	} else {
		return;
	}
	check_callable_function(fci1 TSRMLS_CC);
    efree(args);
}

void pre_global_array_diff_ukey(INTERNAL_FUNCTION_PARAMETERS)
{
    pre_php_array_diff(INTERNAL_FUNCTION_PARAM_PASSTHRU, DIFF_KEY, DIFF_COMP_DATA_INTERNAL, DIFF_COMP_KEY_USER);
}

void pre_global_array_filter(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *array;
	long use_type = 0;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|fl", &array, &fci, &fci_cache, &use_type) == FAILURE) {
		return;
	}
    if (ZEND_NUM_ARGS() > 1) {
		check_callable_function(fci TSRMLS_CC);
	}
}

void pre_global_array_map(INTERNAL_FUNCTION_PARAMETERS)
{
    zval ***arrays = NULL;
	int n_arrays = 0;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!+", &fci, &fci_cache, &arrays, &n_arrays) == FAILURE) {
		return;
	}
	check_callable_function(fci TSRMLS_CC);
    efree(arrays);
}

void pre_global_array_walk(INTERNAL_FUNCTION_PARAMETERS)
{
    HashTable *array;
	zval *userdata = NULL;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hf|z/", &array, &fci, &fci_cache, &userdata) == FAILURE) {
		return;
	}
	check_callable_function(fci TSRMLS_CC);
}

static void pre_base_sort(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *array;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "af", &array, &fci, &fci_cache) == FAILURE) {
		return;
	}
	check_callable_function(fci TSRMLS_CC);
}

void pre_global_uasort(INTERNAL_FUNCTION_PARAMETERS)
{
    pre_base_sort(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_uksort(INTERNAL_FUNCTION_PARAMETERS)
{
    pre_base_sort(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_usort(INTERNAL_FUNCTION_PARAMETERS)
{
    pre_base_sort(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_reflectionfunction___construct(INTERNAL_FUNCTION_PARAMETERS)
{
	if (openrasp_check_type_ignored(ZEND_STRL("callable") TSRMLS_CC))
    {
        return;
    }
    zval *name;
    zval *closure = NULL;
    char *lcname;
    zend_function *fptr;
    char *name_str;
    int name_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name_str, &name_len) == SUCCESS)
    {
        char *nsname;

        lcname = zend_str_tolower_dup(name_str, name_len);

        /* Ignore leading "\" */
        nsname = lcname;
        if (lcname[0] == '\\')
        {
            nsname = &lcname[1];
            name_len--;
        }

        if (zend_hash_find(EG(function_table), nsname, name_len + 1, (void **)&fptr) == FAILURE)
        {
            efree(lcname);
            return;
        }
        if (openrasp_check_callable_black(nsname, name_len TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            ZVAL_STRING(attack_params, nsname, 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            char *message_str = NULL;
            spprintf(&message_str, 0, _("Webshell detected: using '%s' function"), nsname);
            ZVAL_STRING(plugin_message, message_str, 1);
            efree(message_str);
            openrasp_buildin_php_risk_handle(1, "callable", 100, attack_params, plugin_message TSRMLS_CC);
        }
        efree(lcname);
    }
}