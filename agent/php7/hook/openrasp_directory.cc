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

/**
 * directory相关hook点
 */
PRE_HOOK_FUNCTION(dir, DIRECTORY);
PRE_HOOK_FUNCTION(opendir, DIRECTORY);
PRE_HOOK_FUNCTION(scandir, DIRECTORY);

static inline void _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
	if (!isolate)
	{
		return;
	}

	zend_string *dirname;

	if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "P", &dirname) != SUCCESS)
	{
		return;
	}

	if (ZSTR_LEN(dirname) < 1)
	{
		return;
	}

	php_stream *dirp;
	dirp = php_stream_opendir(ZSTR_VAL(dirname), IGNORE_PATH, NULL);
	if (!dirp)
	{
		return;
	}
	php_stream_close(dirp);
	std::string real_path = openrasp_real_path(ZSTR_VAL(dirname), ZSTR_LEN(dirname), false, OPENDIR);
	if (real_path.empty())
	{
		return;
	}

	openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
	{
		v8::HandleScope handle_scope(isolate);
		auto arr = format_debug_backtrace_arr();
		size_t len = arr.size();
		auto stack = v8::Array::New(isolate, len);
		for (size_t i = 0; i < len; i++)
		{
			stack->Set(i, openrasp::NewV8String(isolate, arr[i]));
		}
		auto params = v8::Object::New(isolate);
		params->Set(openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, dirname->val, dirname->len));
		params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path));
		params->Set(openrasp::NewV8String(isolate, "stack"), stack);
		check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
	}
	if (check_result == openrasp::CheckResult::kBlock)
	{
		handle_block();
	}
}

void pre_global_dir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	_hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_opendir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	_hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void pre_global_scandir_DIRECTORY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	_hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}