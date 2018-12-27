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
#include "agent/shared_config_manager.h"

PRE_HOOK_FUNCTION(putenv, WEBSHELL_LD_PRELOAD);

void pre_global_putenv_WEBSHELL_LD_PRELOAD(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *env;
    size_t env_len;
    if (zend_parse_parameters(MIN(1, ZEND_NUM_ARGS()) TSRMLS_CC, "s", &env, &env_len) == FAILURE)
    {
        return;
    }
    if (!openrasp_check_type_ignored(check_type TSRMLS_CC) &&
        env != nullptr &&
        strncmp(env, "LD_PRELOAD=", sizeof("LD_PRELOAD=") - 1) == 0)
    {
        zval *attack_params = NULL;
        MAKE_STD_ZVAL(attack_params);
        array_init(attack_params);
        add_assoc_stringl(attack_params, "env", env, env_len, 1);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        ZVAL_STRING(plugin_message, _("WebShell activity - Detected LD_PRELOAD"), 1);
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(check_type);
        openrasp_buildin_php_risk_handle(action, check_type, 100, attack_params, plugin_message TSRMLS_CC);
    }
}
