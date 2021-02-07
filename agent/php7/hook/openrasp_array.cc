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

#include "hook/checker/builtin_detector.h"
#include "hook/data/callable_object.h"
#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "agent/shared_config_manager.h"

/**
 * callable相关hook点
 */
PRE_HOOK_FUNCTION(array_walk, CALLABLE);
PRE_HOOK_FUNCTION(array_map, CALLABLE);
PRE_HOOK_FUNCTION(array_filter, CALLABLE);

PRE_HOOK_FUNCTION_EX(__construct, reflectionfunction, CALLABLE);

static void check_callable_function(zend_fcall_info fci, OpenRASPCheckType check_type)
{
	if (!ZEND_FCI_INITIALIZED(fci))
	{
		return;
	}
	zval function_name = fci.function_name;
	openrasp::data::CallableObject callable_obj(&function_name, OPENRASP_HOOK_G(callable_blacklist));
	openrasp::checker::BuiltinDetector builtin_detector(callable_obj);
	builtin_detector.run();
}

void pre_global_array_filter_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	if (ZEND_NUM_ARGS() > 1)
	{
		zval *array = nullptr;
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

		if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "af", &array, &fci, &fci_cache) != SUCCESS)
		{
			return;
		}
		check_callable_function(fci, check_type);
	}
}

void pre_global_array_walk_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	pre_global_array_filter_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_array_map_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

	// ZEND_PARSE_PARAMETERS_START(2, -1)
	// 	Z_PARAM_FUNC_EX(fci, fci_cache, 1, 0)
	// 	Z_PARAM_VARIADIC('+', arrays, n_arrays)
	// ZEND_PARSE_PARAMETERS_END();

	if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "f!", &fci, &fci_cache) != SUCCESS)
	{
		return;
	}

	check_callable_function(fci, check_type);
}

void pre_reflectionfunction___construct_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *closure = nullptr;
	zval *name_str = nullptr;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "o", &closure) == SUCCESS)
	{
		return;
	}
	else
	{
		if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z", &name_str) == SUCCESS)
		{
			openrasp::data::CallableObject callable_obj(name_str, OPENRASP_HOOK_G(callable_blacklist));
			openrasp::checker::BuiltinDetector builtin_detector(callable_obj);
			builtin_detector.run();
		}
	}
}