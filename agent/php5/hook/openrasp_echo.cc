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
#include "utils/regex.h"

static bool echo_parameter_filter(const zval *inc_filename TSRMLS_DC);

int echo_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    zend_op *opline = execute_data->opline;
    std::string name;
    std::string var_type;
    if (!openrasp_check_type_ignored(XSS_ECHO TSRMLS_CC) &&
        OPENRASP_OP1_TYPE(opline) == IS_VAR &&
        !(name = fetch_name_in_request(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr, var_type TSRMLS_CC)).empty() &&
        echo_parameter_filter(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr TSRMLS_CC))
    {
        zval *attack_params;
        MAKE_STD_ZVAL(attack_params);
        array_init(attack_params);
        add_assoc_string(attack_params, "type", const_cast<char *>(var_type.c_str()), 1);
        add_assoc_string(attack_params, "name", const_cast<char *>(name.c_str()), 1);
        add_assoc_zval(attack_params, "value", OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr);
        Z_ADDREF_P(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        char *message_str = NULL;
        spprintf(&message_str, 0, _("XSS activity - echo GET/POST/COOKIE parameter directly, parameter: $%s['%s']"), var_type.c_str(), name.c_str());
        ZVAL_STRING(plugin_message, message_str, 1);
        efree(message_str);
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_ECHO);
        openrasp_buildin_php_risk_handle(action, XSS_ECHO, 100, attack_params, plugin_message TSRMLS_CC);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

static bool echo_parameter_filter(const zval *inc_filename TSRMLS_DC)
{
    if (Z_TYPE_P(inc_filename) == IS_STRING &&
        (OPENRASP_CONFIG(xss.echo_filter_regex).empty() ||
         openrasp::regex_search(Z_STRVAL_P(inc_filename), OPENRASP_CONFIG(xss.echo_filter_regex).c_str())))
    {
        return true;
    }
    return false;
}