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

static inline void _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
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
	zend_string *real_path = openrasp_real_path(ZSTR_VAL(dirname), ZSTR_LEN(dirname), false, OPENDIR);
	if (!real_path)
	{
		return;
	}

	v8::Isolate *isolate = openrasp::get_isolate();
	if (!isolate)
	{
		zend_string_release(real_path);
		return;
	}
	bool is_block = false;
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
		params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path->val, real_path->len));
		zend_string_release(real_path);
		params->Set(openrasp::NewV8String(isolate, "stack"), stack);
		is_block = openrasp::openrasp_check(isolate, openrasp::NewV8String(isolate, check_type), params);
	}
	if (is_block)
	{
		handle_block();
	}
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
	_hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}