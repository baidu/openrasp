/*
 * Copyright 2017-2020 Baidu Inc.
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
#include "openrasp_v8.h"
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

static void check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **command;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (argc == 1 && zend_get_parameters_ex(argc, &command) == SUCCESS)
    {
        openrasp::data::CommandObject cmd_obj(*command);
        openrasp::checker::BuiltinDetector builtin_detector(cmd_obj);
        builtin_detector.run();
    }
}

static void plugin_command_check(zval *command TSRMLS_DC)
{
    openrasp::data::CommandObject cmd_obj(command);
    openrasp::checker::V8Detector v8_detector(cmd_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

static void openrasp_exec_ex(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
    zval *cmd = nullptr;
    zval *ret_code = nullptr, *ret_array = nullptr;
    int ret = 0;
    if (mode)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z/", &cmd, &ret_code) == FAILURE)
        {
            return;
        }
    }
    else
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z/z/", &cmd, &ret_array, &ret_code) == FAILURE)
        {
            return;
        }
    }
    plugin_command_check(cmd TSRMLS_CC);
}

void pre_global_passthru_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_passthru_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}

void pre_global_system_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_system_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}

void pre_global_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    openrasp_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

void pre_global_shell_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_shell_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &command) == FAILURE)
    {
        return;
    }
    plugin_command_check(command TSRMLS_CC);
}

void pre_global_proc_open_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_proc_open_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;
    zval *descriptorspec = nullptr;
    zval *pipes = nullptr;
    zval *cwd = nullptr;
    zval *environment = nullptr;
    zval *other_options = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zaz|z!z!z!", &command,
                              &descriptorspec, &pipes, &cwd, &environment,
                              &other_options) == FAILURE)
    {
        return;
    }
    plugin_command_check(command TSRMLS_CC);
}

void pre_global_popen_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_popen_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *command = nullptr;
    char *mode = nullptr;
    int mode_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs", &command, &mode, &mode_len) == FAILURE)
    {
        return;
    }
    plugin_command_check(command TSRMLS_CC);
}

void pre_global_pcntl_exec_WEBSHELL_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    check_command_in_gpc(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_pcntl_exec_COMMAND(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *args = nullptr, *envs = nullptr;
    zval **element;
    HashTable *args_hash = nullptr;
    int argc = 0, argi = 0;
    char **argv = nullptr;
    char **current_arg;
    char *path = nullptr;
    int path_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|aa", &path, &path_len, &args, &envs) == FAILURE)
    {
        return;
    }

    std::string command(path, path_len);
    if (nullptr != args)
    {
        if (IS_ARRAY == Z_TYPE_P(args))
        {
            zval retval;
            zval *z_glue = nullptr;
            MAKE_STD_ZVAL(z_glue);
            ZVAL_STRING(z_glue, " ", 1);
            zval *params[2];
            params[0] = z_glue;
            params[1] = args;
            if (openrasp_call_user_function(EG(function_table), nullptr, "implode", &retval, 2, params TSRMLS_CC) &&
                Z_TYPE(retval) == IS_STRING)
            {
                command.append(" ").append(Z_STRVAL(retval), Z_STRLEN(retval));
                zval_dtor(&retval);
            }
            zval_ptr_dtor(&z_glue);
        }
    }
    zval *z_cmd = nullptr;
    MAKE_STD_ZVAL(z_cmd);
    ZVAL_STRING(z_cmd, command.c_str(), 1);
    plugin_command_check(z_cmd TSRMLS_CC);
    zval_ptr_dtor(&z_cmd);
}