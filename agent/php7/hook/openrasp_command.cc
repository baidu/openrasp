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
    zval *command;

    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "z", &command) != SUCCESS ||
        Z_TYPE_P(command) != IS_STRING)
    {
        return;
    }

    if (openrasp_zval_in_request(command))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "command", command);
        Z_ADDREF_P(command);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("WebShell activity - Detected command execution backdoor"));
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(check_type);
        openrasp_buildin_php_risk_handle(action, check_type, 100, &attack_params, &plugin_message);
    }
}

static inline void plugin_command_check(const zend_string *command, OpenRASPCheckType check_type)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (!isolate)
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
        params->Set(openrasp::NewV8String(isolate, "command"), openrasp::NewV8String(isolate, command->val, command->len));
        params->Set(openrasp::NewV8String(isolate, "stack"), stack);
        check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
    }
    if (check_result == openrasp::CheckResult::kBlock)
    {
        handle_block();
    }
}

static inline void openrasp_command_common(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zend_string *command;

    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()), "S", &command) != SUCCESS)
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
    zend_string *command;
    zval *args;

    if (zend_parse_parameters(MIN(2, ZEND_NUM_ARGS()), "S|a", &command, &args) != SUCCESS)
    {
        return;
    }

    if (ZEND_NUM_ARGS() > 1)
    {
        zend_string *delim = zend_string_init(" ", 1, 0);
        zval rst;
        php_implode(delim, args, &rst);
        zend_string_release(delim);
        if (Z_TYPE(rst) == IS_STRING)
        {
            zend_string *tmp = strpprintf(0, "%s %s", ZSTR_VAL(command), Z_STRVAL(rst));
            if (tmp)
            {
                command = tmp;
                zend_string_delref(command);
            }
        }
    }

    plugin_command_check(command, check_type);
}