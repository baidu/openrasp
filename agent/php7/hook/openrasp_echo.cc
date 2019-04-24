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
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}

static zend_free_op should_free;

static bool echo_parameter_filter(const zval *inc_filename);

int echo_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
#if (PHP_MINOR_VERSION < 3)
    zval *inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#else
    zval *inc_filename = zend_get_zval_ptr(opline, opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#endif
    std::string name;
    std::string var_type;
    if (inc_filename != nullptr &&
        !openrasp_check_type_ignored(XSS_ECHO) &&
        !(name = fetch_name_in_request(inc_filename, var_type)).empty() &&
        echo_parameter_filter(inc_filename))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_string(&attack_params, "type", const_cast<char *>(var_type.c_str()));
        add_assoc_string(&attack_params, "name", const_cast<char *>(name.c_str()));
        add_assoc_zval(&attack_params, "value", inc_filename);
        Z_TRY_ADDREF_P(inc_filename);
        zval plugin_message;
        ZVAL_STR(&plugin_message, strpprintf(0, _("XSS activity - echo GET/POST/COOKIE parameter directly, parameter: $%s['%s']"), var_type.c_str(), name.c_str()));
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(XSS_ECHO);
        openrasp_buildin_php_risk_handle(action, XSS_ECHO, 100, &attack_params, &plugin_message);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

static bool echo_parameter_filter(const zval *inc_filename)
{
    if (Z_TYPE_P(inc_filename) == IS_STRING &&
        (OPENRASP_CONFIG(xss.echo_filter_regex).empty() ||
         openrasp::regex_search(Z_STRVAL_P(inc_filename), OPENRASP_CONFIG(xss.echo_filter_regex).c_str())))
    {
        return true;
    }
    return false;
}