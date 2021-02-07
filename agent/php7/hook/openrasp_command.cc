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

#include "hook/data/command_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/checker/builtin_detector.h"
#include "openrasp_hook.h"
#include "agent/shared_config_manager.h"

/**
 * command相关hook点
 */
PRE_HOOK_FUNCTION(passthru, COMMAND);
PRE_HOOK_FUNCTION(system, COMMAND);
PRE_HOOK_FUNCTION(exec, COMMAND);
PRE_HOOK_FUNCTION(shell_exec, COMMAND);
PRE_HOOK_FUNCTION(proc_open, COMMAND);
PRE_HOOK_FUNCTION(popen, COMMAND);
PRE_HOOK_FUNCTION(pcntl_exec, COMMAND);

PRE_HOOK_FUNCTION(passthru, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(system, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(exec, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(shell_exec, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(proc_open, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(popen, WEBSHELL_COMMAND);
PRE_HOOK_FUNCTION(pcntl_exec, WEBSHELL_COMMAND);

static inline void openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;

    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "z", &command) != SUCCESS ||
        Z_TYPE_P(command) != IS_STRING)
    {
        return;
    }
    openrasp::data::CommandObject cmd_obj(command);
    openrasp::checker::BuiltinDetector builtin_detector(cmd_obj);
    builtin_detector.run();
}

static inline void plugin_command_check(zval *command, OpenRASPCheckType check_type)
{
    openrasp::data::CommandObject cmd_obj(command);
    openrasp::checker::V8Detector v8_detector(cmd_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

static inline void openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;

    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "z", &command) != SUCCESS)
    {
        return;
    }

    plugin_command_check(command, check_type);
}

void pre_global_passthru_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_passthru_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_system_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_system_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_shell_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_shell_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_proc_open_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_proc_open_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_popen_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_popen_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_pcntl_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_webshell_command_common(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_pcntl_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;
    zval *args = nullptr;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "z|a", &command, &args) != SUCCESS)
    {
        return;
    }

    if (ZEND_NUM_ARGS() > 1)
    {
        zend_string *delim = zend_string_init(" ", 1, 0);
        zval rst;
        php_implode(delim, args, &rst);
        zend_string_release(delim);
        if (Z_TYPE(rst) == IS_STRING && Z_TYPE_P(command) == IS_STRING)
        {
            zval complete_cmd;
            ZVAL_STR(&complete_cmd, strpprintf(0, _("%s %s"), Z_STRVAL_P(command), Z_STRVAL(rst)));
            plugin_command_check(&complete_cmd, check_type);
            zval_ptr_dtor(&complete_cmd);
        }
    }
    else
    {
        plugin_command_check(command, check_type);
    }
}