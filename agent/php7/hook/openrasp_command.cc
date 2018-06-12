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
 * command相关hook点
 */
PRE_HOOK_FUNCTION(passthru, command);
PRE_HOOK_FUNCTION(system, command);
PRE_HOOK_FUNCTION(exec, command);
PRE_HOOK_FUNCTION(shell_exec, command);
PRE_HOOK_FUNCTION(proc_open, command);
PRE_HOOK_FUNCTION(popen, command);
PRE_HOOK_FUNCTION(pcntl_exec, command);

PRE_HOOK_FUNCTION(passthru, webshell_command);
PRE_HOOK_FUNCTION(system, webshell_command);
PRE_HOOK_FUNCTION(exec, webshell_command);
PRE_HOOK_FUNCTION(shell_exec, webshell_command);
PRE_HOOK_FUNCTION(proc_open, webshell_command);
PRE_HOOK_FUNCTION(popen, webshell_command);
PRE_HOOK_FUNCTION(pcntl_exec, webshell_command);

static void check_command_args_in_gpc(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (argc == 1 &&
        zend_get_parameters_ex(argc, &command) == SUCCESS &&
        openrasp_zval_in_request(command TSRMLS_CC))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "command", command);
        Z_ADDREF_P(command);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("Webshell detected - Command execution backdoor"));
        openrasp_buildin_php_risk_handle(1, "webshell_command", 100, &attack_params, &plugin_message TSRMLS_CC);
    }
}

static void send_command_to_plugin(char *cmd TSRMLS_DC)
{
    zval params;
    array_init(&params);
    add_assoc_string(&params, "command", cmd);
    zval stack;
    array_init(&stack);
    format_debug_backtrace_arr(&stack TSRMLS_CC);
    add_assoc_zval(&params, "stack", &stack);
    check("command", &params TSRMLS_CC);
}

static void openrasp_exec_ex(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
    char *cmd;
    size_t cmd_len;
    zval *ret_code = NULL, *ret_array = NULL;
    int ret;

    if (mode)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|z/", &cmd, &cmd_len, &ret_code) == FAILURE)
        {
            return;
        }
    }
    else
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|z/z/", &cmd, &cmd_len, &ret_array, &ret_code) == FAILURE)
        {
            return;
        }
    }

    if (!cmd_len)
    {
        return;
    }

    send_command_to_plugin(cmd TSRMLS_CC);
}

void pre_global_passthru_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_passthru_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}

void pre_global_system_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_system_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}

void pre_global_exec_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_exec_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

void pre_global_shell_exec_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_shell_exec_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *command;
    size_t command_len;

    // ZEND_PARSE_PARAMETERS_START(1, 1)
    // Z_PARAM_STRING(command, command_len)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &command, &command_len) == FAILURE) {
		return;
	}

    if (!command_len)
    {
        return;
    }
    send_command_to_plugin(command TSRMLS_CC);
}

void pre_global_proc_open_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_proc_open_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *command, *cwd = NULL;
    size_t command_len, cwd_len = 0;
    zval *descriptorspec;
    zval *pipes;
    zval *environment = NULL;
    zval *other_options = NULL;

    // ZEND_PARSE_PARAMETERS_START(3, 6)
    // Z_PARAM_STRING(command, command_len)
    // Z_PARAM_ARRAY(descriptorspec)
    // Z_PARAM_ZVAL_DEREF(pipes)
    // Z_PARAM_OPTIONAL
    // Z_PARAM_STRING_EX(cwd, cwd_len, 1, 0)
    // Z_PARAM_ARRAY_EX(environment, 1, 0)
    // Z_PARAM_ARRAY_EX(other_options, 1, 0)
    // ZEND_PARSE_PARAMETERS_END_EX(return);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "saz|s!a!a!", &command,
				&command_len, &descriptorspec, &pipes, &cwd, &cwd_len, &environment,
				&other_options) == FAILURE) {
		return;
	}

    if (!command_len)
    {
        return;
    }
    send_command_to_plugin(command TSRMLS_CC);
}

void pre_global_popen_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_popen_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *command, *mode;
    size_t command_len, mode_len;

    // ZEND_PARSE_PARAMETERS_START(2, 2)
    // Z_PARAM_PATH(command, command_len)
    // Z_PARAM_STRING(mode, mode_len)
    // ZEND_PARSE_PARAMETERS_END();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &command, &command_len, &mode, &mode_len) == FAILURE) {
		return;
	}

    if (!command_len)
    {
        return;
    }
    send_command_to_plugin(command TSRMLS_CC);
}

void pre_global_pcntl_exec_webshell_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_args_in_gpc(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_pcntl_exec_command(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *args = NULL, *envs = NULL;
    HashTable *args_hash;
    int argc = 0, argi = 0;
    char *path;
    size_t path_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|aa", &path, &path_len, &args, &envs) == FAILURE)
    {
        return;
    }
    zval command;
    if (ZEND_NUM_ARGS() > 1)
    {
        zval args_str;
        zend_string *delim = zend_string_init(ZEND_STRL(" "), 0);
        php_implode(delim, args, &args_str TSRMLS_CC);
        zend_string_release(delim);
        char *cmd = nullptr;
        spprintf(&cmd, 0, "%s %s", path, Z_STRVAL(args_str));
        ZVAL_STRING(&command, cmd);
        efree(cmd);
        zval_ptr_dtor(&args_str);
    }
    else
    {
        ZVAL_STRINGL(&command, path, path_len);
    }
    zval params;
    array_init(&params);
    add_assoc_zval(&params, "command", &command);
    zval stack;
    array_init(&stack);
    format_debug_backtrace_arr(&stack TSRMLS_CC);
    add_assoc_zval(&params, "stack", &stack);
    check("command", &params TSRMLS_CC);
}