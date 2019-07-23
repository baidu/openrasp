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
#include "utils/string.h"
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}

static std::string parse_parameter_to_string(zend_execute_data *execute_data);
int include_handler(zend_execute_data *execute_data);
int eval_handler(zend_execute_data *execute_data);
int include_or_eval_handler(zend_execute_data *execute_data);
static zend_free_op should_free;

int include_or_eval_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
    if (opline->extended_value == ZEND_EVAL)
    {
        return eval_handler(execute_data);
    }
    else
    {
        return include_handler(execute_data);
    }
}
int eval_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
#if (PHP_MINOR_VERSION < 3)
    zval *inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#else
    zval *inc_filename = zend_get_zval_ptr(opline, opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
#endif
    if (inc_filename != nullptr &&
        !openrasp_check_type_ignored(WEBSHELL_EVAL) &&
        openrasp_zval_in_request(inc_filename))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "eval", inc_filename);
        Z_TRY_ADDREF_P(inc_filename);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("WebShell activity - Detected China Chopper webshell (eval method)"));
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(WEBSHELL_EVAL);
        openrasp_buildin_php_risk_handle(action, WEBSHELL_EVAL, 100, &attack_params, &plugin_message);
    }
    std::string param = parse_parameter_to_string(execute_data);
    if (!openrasp_check_type_ignored(EVAL) && !param.empty())
    {
        openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
        openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
        if (isolate)
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            params->Set(openrasp::NewV8String(isolate, "code"), openrasp::NewV8String(isolate, param));
            params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "eval"));
            check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(EVAL)), params, OPENRASP_CONFIG(plugin.timeout.millis));
        }
        if (check_result == openrasp::CheckResult::kBlock)
        {
            handle_block();
        }
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

static std::string parse_parameter_to_string(zend_execute_data *execute_data)
{
    std::string param;
    const zend_op *opline = EX(opline);
    zval tmp_inc_filename, *inc_filename;
    ZVAL_NULL(&tmp_inc_filename);
#if (PHP_MINOR_VERSION < 3)
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
        param = std::string(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename));
    }
    if (Z_TYPE(tmp_inc_filename) != IS_NULL)
    {
        zval_ptr_dtor(&tmp_inc_filename);
    }
    return param;
}

int include_handler(zend_execute_data *execute_data)
{
    if (openrasp_check_type_ignored(INCLUDE))
    {
        return ZEND_USER_OPCODE_DISPATCH;
    }
    zval params, *document_root;
    std::string real_path;
    const char *scheme_end = nullptr;
    bool send_to_plugin = false;
    std::string param = parse_parameter_to_string(execute_data);
    if (!param.empty())
    {
        if (((scheme_end = fetch_url_scheme(param.c_str())) != nullptr) ||
            (param.length() < 4 || (!openrasp::end_with(param, ".php") && !openrasp::end_with(param, ".inc"))))
        {
            real_path = openrasp_real_path(param.c_str(), param.length(), true, READING);
        }
        if (!real_path.empty())
        {
            if (scheme_end || param.find("../") != std::string::npos)
            {
                send_to_plugin = true;
            }
            if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
            {
                send_to_plugin = true;
            }
            document_root = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRL("DOCUMENT_ROOT"));
            if (nullptr == document_root)
            {
                send_to_plugin = true;
            }
            else
            {
                assert(Z_TYPE_P(document_root) == IS_STRING);
                if (strncmp(real_path.c_str(), Z_STRVAL_P(document_root), Z_STRLEN_P(document_root)) == 0)
                {
                    send_to_plugin = false;
                }
                else
                {
                    send_to_plugin = true;
                }
            }
            openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            if (send_to_plugin && isolate)
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                auto path = openrasp::NewV8String(isolate, param);
                params->Set(openrasp::NewV8String(isolate, "path"), path);
                params->Set(openrasp::NewV8String(isolate, "url"), path);
                params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path));
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
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function));
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(INCLUDE)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block();
            }
        }
    }
    return ZEND_USER_OPCODE_DISPATCH;
}