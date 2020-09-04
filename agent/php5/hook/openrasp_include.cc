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

#include "hook/data/eval_object.h"
#include "hook/data/include_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/checker/builtin_detector.h"
#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "agent/shared_config_manager.h"
#include "utils/string.h"

PRE_HOOK_FUNCTION(assert, EVAL);
PRE_HOOK_FUNCTION(assert, WEBSHELL_EVAL);

void include_handler(zval *op1, ZEND_OPCODE_HANDLER_ARGS);
void eval_handler(zval *op1, ZEND_OPCODE_HANDLER_ARGS);

int include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    zend_op *opline = execute_data->opline;
    zval *op1 = nullptr;
    zval *z_param = nullptr;
    switch (OPENRASP_OP1_TYPE(opline))
    {
    case IS_TMP_VAR:
        op1 = &OPENRASP_T(OPENRASP_OP1_VAR(opline)).tmp_var;
        break;
    case IS_VAR:
        op1 = OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr;
        break;
    case IS_CV:
    {
        zval **t = OPENRASP_CV(OPENRASP_OP1_VAR(opline));
        if (t && *t)
        {
            op1 = *t;
        }
        else if (EG(active_symbol_table))
        {
            zend_compiled_variable *cv = &EG(active_op_array)->vars[OPENRASP_OP1_VAR(opline)];
            if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1, cv->hash_value, (void **)&t) == SUCCESS)
            {
                op1 = *t;
            }
        }
        break;
    }
    case IS_CONST:
        op1 = OPENRASP_OP1_CONSTANT_PTR(opline);
        break;
    }
    if (op1->type != IS_STRING)
    {
        MAKE_STD_ZVAL(z_param);
        ZVAL_COPY_VALUE(z_param, op1);
        zval_copy_ctor(z_param);
        convert_to_string(z_param);
        op1 = z_param;
    }
    if (OPENRASP_INCLUDE_OR_EVAL_TYPE(execute_data->opline) == ZEND_EVAL)
    {
        eval_handler(op1, ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
    }
    else
    {
        include_handler(op1, ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
    }
    if (nullptr != z_param)
    {
        zval_ptr_dtor(&z_param);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

void eval_handler(zval *op1, ZEND_OPCODE_HANDLER_ARGS)
{
    openrasp::data::EvalObject eval_obj(op1, "eval");
    if (!openrasp_check_type_ignored(EVAL TSRMLS_CC))
    {
        openrasp::checker::V8Detector v8_detector(eval_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
        v8_detector.run();
    }
    if (!openrasp_check_type_ignored(WEBSHELL_EVAL TSRMLS_CC))
    {
        openrasp::checker::BuiltinDetector builtin_detector(eval_obj);
        builtin_detector.run();
    }
}

void include_handler(zval *op1, ZEND_OPCODE_HANDLER_ARGS)
{
    std::string function;
    switch (OPENRASP_INCLUDE_OR_EVAL_TYPE(execute_data->opline))
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
        if (!openrasp_check_type_ignored(SSRF TSRMLS_CC))
        {
            plugin_ssrf_check(op1, function TSRMLS_CC);
        }
    }
    else
    {
        if (!openrasp_check_type_ignored(INCLUDE TSRMLS_CC))
        {
            openrasp::data::IncludeObject include_obj(op1, OPENRASP_G(request).get_document_root(), function, OPENRASP_CONFIG(plugin.filter), protocol.empty());
            openrasp::checker::V8Detector v8_detector(include_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
            v8_detector.run();
        }
    }
}

void pre_global_assert_WEBSHELL_EVAL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **assertion;
    int description_len = 0;
    char *description = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|s", &assertion, &description, &description_len) == FAILURE)
    {
        return;
    }
    openrasp::data::EvalObject eval_obj(*assertion, "assert");
    openrasp::checker::BuiltinDetector builtin_detector(eval_obj);
    builtin_detector.run();
}

void pre_global_assert_EVAL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval **assertion;
    int description_len = 0;
    char *description = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|s", &assertion, &description, &description_len) == FAILURE)
    {
        return;
    }

    openrasp::data::EvalObject eval_obj(*assertion, "assert");
    openrasp::checker::V8Detector v8_detector(eval_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}