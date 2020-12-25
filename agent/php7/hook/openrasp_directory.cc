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

#include "hook/data/file_op_object.h"
#include "hook/checker/v8_detector.h"
#include "openrasp_hook.h"

/**
 * directory相关hook点
 */
PRE_HOOK_FUNCTION(dir, DIRECTORY);
PRE_HOOK_FUNCTION(opendir, DIRECTORY);
PRE_HOOK_FUNCTION(scandir, DIRECTORY);

static inline void _hook_php_do_opendir(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *dirname;

	if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "z", &dirname) != SUCCESS)
	{
		return;
	}

	openrasp::data::FileOpObject dir_obj(dirname, OPENDIR);
	openrasp::checker::V8Detector v8_detector(dir_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
	v8_detector.run();
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