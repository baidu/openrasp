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

#include "hook/checker/builtin_detector.h"
#include "hook/data/echo_object.h"
#include "openrasp_hook.h"
#include "agent/shared_config_manager.h"

int echo_print_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    zend_op *opline = execute_data->opline;
    if (!openrasp_check_type_ignored(XSS_ECHO TSRMLS_CC) &&
        OPENRASP_OP1_TYPE(opline) == IS_VAR &&
        openrasp_zval_in_request(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr))
    {
        std::string opname = (opline->opcode == ZEND_ECHO) ? "echo" : "print";
        openrasp::data::EchoObject echo_obj(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr, opname, OPENRASP_HOOK_G(echo_filter_regex));
        openrasp::checker::BuiltinDetector builtin_detector(echo_obj);
        builtin_detector.run();
    }
    return ZEND_USER_OPCODE_DISPATCH;
}