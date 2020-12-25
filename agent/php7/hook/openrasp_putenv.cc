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

#include "hook/data/putenv_object.h"
#include "hook/checker/builtin_detector.h"
#include "openrasp_hook.h"
#include "agent/shared_config_manager.h"

PRE_HOOK_FUNCTION(putenv, WEBSHELL_ENV);

void pre_global_putenv_WEBSHELL_ENV(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *env = nullptr;
    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "z", &env) == FAILURE)
    {
        return;
    }
    openrasp::data::PutenvObject putenv_obj(env);
    openrasp::checker::BuiltinDetector builtin_detector(putenv_obj);
    builtin_detector.run();
}
