/*
 * Copyright 2017-2021 Baidu Inc.
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
#include "utils/regex.h"
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}

static zend_free_op should_free;

int echo_print_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3)
    zval *inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#else
    zval *inc_filename = zend_get_zval_ptr(opline, opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#endif
    if (inc_filename != nullptr &&
        !openrasp_check_type_ignored(XSS_ECHO) &&
        openrasp_zval_in_request(inc_filename))
    {
        std::string opname = (opline->extended_value == 0) ? "echo" : "print";
        openrasp::data::EchoObject echo_obj(inc_filename, opname, OPENRASP_HOOK_G(echo_filter_regex));
        openrasp::checker::BuiltinDetector builtin_detector(echo_obj);
        builtin_detector.run();
    }
    return ZEND_USER_OPCODE_DISPATCH;
}