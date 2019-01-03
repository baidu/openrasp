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
 * callable相关hook点
 */
PRE_HOOK_FUNCTION(array_walk, CALLABLE);
PRE_HOOK_FUNCTION(array_map, CALLABLE);
PRE_HOOK_FUNCTION(array_filter, CALLABLE);

PRE_HOOK_FUNCTION_EX(__construct, reflectionfunction, CALLABLE);

static void check_callable_function(zend_fcall_info fci TSRMLS_DC)
{
    if (!ZEND_FCI_INITIALIZED(fci))
    {
        return;
    }
    zval *function_name = fci.function_name;
    if (Z_TYPE_P(function_name) == IS_STRING && Z_STRLEN_P(function_name) > 0)
    {
        if (openrasp_check_callable_black(Z_STRVAL_P(function_name), Z_STRLEN_P(function_name) TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_string(attack_params, "function", Z_STRVAL_P(function_name), 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            char *message_str = NULL;
            spprintf(&message_str, 0, _("WebShell activity - Using dangerous callback method %s()"), Z_STRVAL_P(function_name));
            ZVAL_STRING(plugin_message, message_str, 1);
            efree(message_str);
            OpenRASPActionType action = openrasp::scm->get_buildin_check_action(CALLABLE);
            openrasp_buildin_php_risk_handle(action, CALLABLE, 100, attack_params, plugin_message TSRMLS_CC);
        }
    }
}

void pre_global_array_filter_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *array;
    long use_type = 0;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|fl", &array, &fci, &fci_cache, &use_type) == FAILURE)
    {
        return;
    }
    if (ZEND_NUM_ARGS() > 1)
    {
        check_callable_function(fci TSRMLS_CC);
    }
}

void pre_global_array_map_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval ***arrays = NULL;
    int n_arrays = 0;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f!+", &fci, &fci_cache, &arrays, &n_arrays) == FAILURE)
    {
        return;
    }
    check_callable_function(fci TSRMLS_CC);
    efree(arrays);
}

void pre_global_array_walk_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    HashTable *array;
    zval *userdata = NULL;
    zend_fcall_info fci;
    zend_fcall_info_cache fci_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hf|z/", &array, &fci, &fci_cache, &userdata) == FAILURE)
    {
        return;
    }
    check_callable_function(fci TSRMLS_CC);
}

void pre_reflectionfunction___construct_CALLABLE(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *name;
    zval *closure = NULL;
    char *lcname;
    zend_function *fptr;
    char *name_str;
    int name_len;

    if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "o", &closure) == SUCCESS)
    {
        return;
    }
    else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name_str, &name_len) == SUCCESS)
    {
        char *nsname;

        lcname = zend_str_tolower_dup(name_str, name_len);

        /* Ignore leading "\" */
        nsname = lcname;
        if (lcname[0] == '\\')
        {
            nsname = &lcname[1];
            name_len--;
        }

        if (zend_hash_find(EG(function_table), nsname, name_len + 1, (void **)&fptr) == FAILURE)
        {
            efree(lcname);
            return;
        }
        if (openrasp_check_callable_black(nsname, name_len TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            array_init(attack_params);
            add_assoc_string(attack_params, "function", nsname, 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            char *message_str = NULL;
            spprintf(&message_str, 0, _("Webshell detected: using '%s' function"), nsname);
            ZVAL_STRING(plugin_message, message_str, 1);
            efree(message_str);
            OpenRASPActionType action = openrasp::scm->get_buildin_check_action(check_type);
            openrasp_buildin_php_risk_handle(action, check_type, 100, attack_params, plugin_message TSRMLS_CC);
        }
        efree(lcname);
    }
}