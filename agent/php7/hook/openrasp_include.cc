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

#include "hook/data/eval_object.h"
#include "hook/data/include_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/checker/builtin_detector.h"
#include "openrasp_hook.h"
#include "agent/shared_config_manager.h"
#include "utils/string.h"
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}

static zend_free_op should_free;
void include_handler(zval *op1, zend_execute_data *execute_data);
void eval_handler(zval *op1, zend_execute_data *execute_data);
int include_or_eval_handler(zend_execute_data *execute_data);

int include_or_eval_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
    zval tmp_inc_filename;
    zval *inc_filename = nullptr;
    ZVAL_NULL(&tmp_inc_filename);
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3)
    inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#else
    inc_filename = zend_get_zval_ptr(opline, opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#endif
    if (inc_filename != nullptr)
    {
        if (Z_TYPE_P(inc_filename) != IS_STRING)
        {
            tmp_inc_filename = *inc_filename;
            zval_copy_ctor(&tmp_inc_filename);
            convert_to_string(&tmp_inc_filename);
            inc_filename = &tmp_inc_filename;
        }
    }
    if (opline->extended_value == ZEND_EVAL)
    {
        eval_handler(inc_filename, execute_data);
    }
    else
    {
        include_handler(inc_filename, execute_data);
    }
    if (Z_TYPE(tmp_inc_filename) != IS_NULL)
    {
        zval_ptr_dtor(&tmp_inc_filename);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

void eval_handler(zval *op1, zend_execute_data *execute_data)
{
    openrasp::data::EvalObject eval_obj(op1, "eval");
    if (!openrasp_check_type_ignored(EVAL))
    {
        openrasp::checker::V8Detector v8_detector(eval_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
        v8_detector.run();
    }
    if (!openrasp_check_type_ignored(WEBSHELL_EVAL))
    {
        openrasp::checker::BuiltinDetector builtin_detector(eval_obj);
        builtin_detector.run();
    }
}

void include_handler(zval *op1, zend_execute_data *execute_data)
{
    std::string function;
    const zend_op *opline = EX(opline);
    switch (opline->extended_value)
    {
    case ZEND_INCLUDE:
        function = "include";
        break;
    case ZEND_INCLUDE_ONCE:
        function = "include_once";
        break;
    case ZEND_REQUIRE:
        function = "require";
        break;
    case ZEND_REQUIRE_ONCE:
        function = "require_once";
        break;
    default:
        break;
    }
    std::string protocol = fetch_possible_protocol(Z_STRVAL_P(op1));

    if (maybe_ssrf_vulnerability(protocol))
    {
        if (!openrasp_check_type_ignored(SSRF))
        {
            plugin_ssrf_check(op1, function);
        }
    }
    else
    {
        if (!openrasp_check_type_ignored(INCLUDE))
        {
            openrasp::data::IncludeObject include_obj(op1, OPENRASP_G(request).get_document_root(), function, OPENRASP_CONFIG(plugin.filter), protocol.empty());
            openrasp::checker::V8Detector v8_detector(include_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
            v8_detector.run();
        }
    }
}