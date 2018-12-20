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
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}

static zend_free_op should_free;

int echo_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
    zval *inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
    if (inc_filename != nullptr &&
        opline->op1_type == IS_VAR &&
        !openrasp_check_type_ignored(XSS_ECHO) &&
        openrasp_zval_in_request(inc_filename))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "echo", inc_filename);
        Z_TRY_ADDREF_P(inc_filename);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("XSS activity - echo GET/POST/COOKIE parameter directly"));
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_ECHO);
        openrasp_buildin_php_risk_handle(action, XSS_ECHO, 100, &attack_params, &plugin_message);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}