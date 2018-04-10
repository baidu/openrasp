#pragma once

#include "openrasp_hook.h"

inline void hook_command(INTERNAL_FUNCTION_PARAMETERS)
{
    zval **command;
    int argc = MIN(1, ZEND_NUM_ARGS());
    if (argc == 1 && zend_get_parameters_ex(argc, &command) == SUCCESS &&
        Z_TYPE_PP(command) == IS_STRING)
    {
        if (!openrasp_check_type_ignored(ZEND_STRL("webshell_command") TSRMLS_CC)
            && openrasp_zval_in_request(*command TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_zval(attack_params, "command", *command);
            Z_ADDREF_PP(command);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            ZVAL_STRING(plugin_message, _("Webshell detected - Command execution backdoor"), 1);
            openrasp_buildin_php_risk_handle(1, "webshell_command", 100, attack_params, plugin_message TSRMLS_CC);
        }
        if (!openrasp_check_type_ignored(ZEND_STRL("command") TSRMLS_CC) && Z_STRLEN_PP(command) > 0)
        {
            zval *params;
            MAKE_STD_ZVAL(params);
            array_init(params);
            add_assoc_zval(params, "command", *command);
            Z_ADDREF_P(*command);
            zval *stack = NULL;
            MAKE_STD_ZVAL(stack);
            array_init(stack);
            format_debug_backtrace_arr(stack TSRMLS_CC);
            add_assoc_zval(params, "stack", stack);
            check("command", params TSRMLS_CC);
        }
    }
}
#define HOOK_COMMAND(name)                                 \
    OPENRASP_HOOK_FUNCTION(name)                           \
    {                                                      \
        hook_command(INTERNAL_FUNCTION_PARAM_PASSTHRU);    \
        origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
    }
HOOK_COMMAND(passthru);
HOOK_COMMAND(system);
HOOK_COMMAND(exec);
HOOK_COMMAND(shell_exec);
HOOK_COMMAND(proc_open);
HOOK_COMMAND(popen);

OPENRASP_HOOK_FUNCTION(pcntl_exec)
{
    zval **path, **args;
    int argc = MIN(2, ZEND_NUM_ARGS());
    if (argc > 0 && zend_get_parameters_ex(argc, &path, &args) == SUCCESS &&
        Z_TYPE_PP(path) == IS_STRING)
    {
        if (!openrasp_check_type_ignored(ZEND_STRL("webshell_command") TSRMLS_CC)
        && openrasp_zval_in_request(*path TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_zval(attack_params, "command", *path);
            Z_ADDREF_PP(path);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            ZVAL_STRING(plugin_message, _("Webshell detected - Command execution backdoor"), 1);
            openrasp_buildin_php_risk_handle(1, "webshell_command", 100, attack_params, plugin_message TSRMLS_CC);
        }
        if (!openrasp_check_type_ignored(ZEND_STRL("command") TSRMLS_CC))
        {
            zval *command;
            if (argc > 1 &&
                Z_TYPE_PP(args) == IS_ARRAY)
            {
                zval *delim, *commands;
                MAKE_STD_ZVAL(command);
                MAKE_STD_ZVAL(delim);
                ZVAL_STRINGL(delim, " ", 1, 1);
                MAKE_STD_ZVAL(commands);
                ZVAL_COPY_VALUE(commands, *args);
                zval_copy_ctor(commands);
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 5)
                php_splice(Z_ARRVAL_P(commands), 0, 0, &path, 1, NULL);
#elif (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION == 6)
                php_splice(Z_ARRVAL_P(commands), 0, 0, &path, 1, NULL TSRMLS_CC);
#endif    
                php_implode(delim, commands, command TSRMLS_CC);
                zval_ptr_dtor(&delim);
            }
            else
            {
                command = *path;
                Z_ADDREF_P(command);
            }
            zval *params;
            MAKE_STD_ZVAL(params);
            array_init(params);
            add_assoc_zval(params, "command", command);
            zval *stack = NULL;
            MAKE_STD_ZVAL(stack);
            array_init(stack);
            format_debug_backtrace_arr(stack TSRMLS_CC);
            add_assoc_zval(params, "stack", stack);
            check("command", params TSRMLS_CC);
        }

    }
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

OPENRASP_HOOK_FUNCTION(assert)
{
    zval **assertion;
	int description_len = 0;
	char *description = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|s", &assertion, &description, &description_len) == FAILURE) {
		return;
	}

	if (Z_TYPE_PP(assertion) == IS_STRING) 
    {
        if (!openrasp_check_type_ignored(ZEND_STRL("webshell_eval") TSRMLS_CC)
        && openrasp_zval_in_request(*assertion TSRMLS_CC))
        {
            zval *attack_params;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_zval(attack_params, "eval", *assertion);
            Z_ADDREF_PP(assertion);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            ZVAL_STRING(plugin_message, _("Webshell detected - China Chopper"), 1);
            openrasp_buildin_php_risk_handle(1, "webshell_eval", 100, attack_params, plugin_message TSRMLS_CC);
        }
    }
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}