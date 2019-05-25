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
#include "openrasp_v8.h"
#include "agent/shared_config_manager.h"

int include_handler(ZEND_OPCODE_HANDLER_ARGS);
int eval_handler(ZEND_OPCODE_HANDLER_ARGS);
int include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    if (OPENRASP_INCLUDE_OR_EVAL_TYPE(execute_data->opline) == ZEND_EVAL)
    {
        return eval_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
    }
    else
    {
        return include_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
    }
}
int eval_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    zend_op *opline = execute_data->opline;
    if (!openrasp_check_type_ignored(WEBSHELL_EVAL TSRMLS_CC) &&
        OPENRASP_OP1_TYPE(opline) == IS_VAR &&
        openrasp_zval_in_request(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr TSRMLS_CC))
    {
        zval *attack_params;
        MAKE_STD_ZVAL(attack_params);
        array_init(attack_params);
        add_assoc_zval(attack_params, "eval", OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr);
        Z_ADDREF_P(OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        ZVAL_STRING(plugin_message, _("WebShell activity - Detected China Chopper webshell (eval method)"), 1);
        OpenRASPActionType action = openrasp::scm->get_buildin_check_action(WEBSHELL_EVAL);
        openrasp_buildin_php_risk_handle(action, WEBSHELL_EVAL, 100, attack_params, plugin_message TSRMLS_CC);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}
int include_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    zend_op *opline = execute_data->opline;
    zval *op1 = nullptr;
    switch (OPENRASP_OP1_TYPE(opline))
    {
    case IS_TMP_VAR:
    {
        op1 = &OPENRASP_T(OPENRASP_OP1_VAR(opline)).tmp_var;
        break;
    }
    case IS_VAR:
    {
        op1 = OPENRASP_T(OPENRASP_OP1_VAR(opline)).var.ptr;
        break;
    }
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
    {
        op1 = OPENRASP_OP1_CONSTANT_PTR(opline);
        break;
    }
    default:
    {
        return ZEND_USER_OPCODE_DISPATCH;
    }
    }
    if (openrasp_check_type_ignored(INCLUDE TSRMLS_CC))
    {
        return ZEND_USER_OPCODE_DISPATCH;
    }
    zval *path;
    MAKE_STD_ZVAL(path);
    MAKE_COPY_ZVAL(&op1, path);
    convert_to_string(path);
    std::string real_path;
    const char *scheme_end = nullptr;
    if ((nullptr != Z_STRVAL_P(path) && (scheme_end = fetch_url_scheme(Z_STRVAL_P(path))) != nullptr) ||
        (strlen(Z_STRVAL_P(path)) < 4 || (strcmp(Z_STRVAL_P(path) + Z_STRLEN_P(path) - 4, ".php") &&
                                          strcmp(Z_STRVAL_P(path) + Z_STRLEN_P(path) - 4, ".inc"))))
    {
        real_path = openrasp_real_path(Z_STRVAL_P(path), Z_STRLEN_P(path), 1, READING TSRMLS_CC);
    }

    if (real_path.empty())
    {
        zval_ptr_dtor(&path);
    }
    else
    {
        zval **doc_root;
        bool send_to_plugin = false;
        if (scheme_end || strstr(Z_STRVAL_P(path), "../") != nullptr)
        {
            send_to_plugin = true;
        }
        if (!PG(http_globals)[TRACK_VARS_SERVER] && !zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
        {
            send_to_plugin = true;
        }
        if (Z_TYPE_P(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY)
        {
            if (zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRS("DOCUMENT_ROOT"), (void **)&doc_root) == FAILURE)
            {
                send_to_plugin = true;
            }
            else
            {
                assert(Z_TYPE_PP(doc_root) == IS_STRING);
                if (0 == strncmp(real_path.c_str(), Z_STRVAL_PP(doc_root), Z_STRLEN_PP(doc_root)))
                {
                    send_to_plugin = false;
                }
                else
                {
                    send_to_plugin = true;
                }
            }
        }
        openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
        if (send_to_plugin && isolate)
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
            openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
            {
                v8::HandleScope handle_scope(isolate);
                auto params = v8::Object::New(isolate);
                params->Set(openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, Z_STRVAL_P(path), Z_STRLEN_P(path)));
                params->Set(openrasp::NewV8String(isolate, "url"), openrasp::NewV8String(isolate, Z_STRVAL_P(path), Z_STRLEN_P(path)));
                zval_ptr_dtor(&path);
                params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path));
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function));
                check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(INCLUDE)), params, OPENRASP_CONFIG(plugin.timeout.millis));
            }
            if (check_result == openrasp::CheckResult::kBlock)
            {
                handle_block(TSRMLS_C);
            }
        }
        else
        {
            //skip
            zval_ptr_dtor(&path);
        }
    }
    return ZEND_USER_OPCODE_DISPATCH;
}