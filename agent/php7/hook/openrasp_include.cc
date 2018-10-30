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
extern "C"
{
#include "Zend/zend_vm_opcodes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_execute.h"
}
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
    zval *inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
    if (inc_filename != nullptr &&
        opline->op1_type == IS_VAR &&
        !openrasp_check_type_ignored(ZEND_STRL("webshell_eval")) &&
        openrasp_zval_in_request(inc_filename))
    {
        zval attack_params;
        array_init(&attack_params);
        add_assoc_zval(&attack_params, "eval", inc_filename);
        Z_TRY_ADDREF_P(inc_filename);
        zval plugin_message;
        ZVAL_STRING(&plugin_message, _("WebShell activity - Detected China Chopper webshell (eval method)"));
        openrasp_buildin_php_risk_handle(1, "webshell_eval", 100, &attack_params, &plugin_message);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}
int include_handler(zend_execute_data *execute_data)
{
    const zend_op *opline = EX(opline);
    zval params, tmp_inc_filename, *inc_filename, *document_root;
    ZVAL_NULL(&tmp_inc_filename);
    zend_string *real_path = nullptr;
    inc_filename = zend_get_zval_ptr(opline->op1_type, &opline->op1, execute_data, &should_free, BP_VAR_IS);
    const char *scheme_end = nullptr;
    bool send_to_plugin = false;
    if (inc_filename == nullptr)
    {
        goto DISPATCH;
    }
    if (openrasp_check_type_ignored(ZEND_STRL("include")))
    {
        goto DISPATCH;
    }
    if (Z_TYPE_P(inc_filename) != IS_STRING)
    {
        tmp_inc_filename = *inc_filename;
        zval_copy_ctor(&tmp_inc_filename);
        convert_to_string(&tmp_inc_filename);
        inc_filename = &tmp_inc_filename;
    }
    if ((Z_STRVAL_P(inc_filename) && (scheme_end = fetch_url_scheme(Z_STRVAL_P(inc_filename))) != nullptr) ||
        (strlen(Z_STRVAL_P(inc_filename)) < 4 || (strcmp(Z_STRVAL_P(inc_filename) + Z_STRLEN_P(inc_filename) - 4, ".php") &&
                                                  strcmp(Z_STRVAL_P(inc_filename) + Z_STRLEN_P(inc_filename) - 4, ".inc"))))
    {
        real_path = openrasp_real_path(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename), true, READING);
    }
    if (!real_path)
    {
        goto DISPATCH;
    }
    if (scheme_end || strstr(Z_STRVAL_P(inc_filename), "../") != nullptr)
    {
        send_to_plugin = true;
    }
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY &&
        !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
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
        if (strncmp(ZSTR_VAL(real_path), Z_STRVAL_P(document_root), Z_STRLEN_P(document_root)) == 0)
        {
            send_to_plugin = false;
        }
        else
        {
            send_to_plugin = true;
        }
    }

    {
        openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
        bool is_block = false;
        if (!send_to_plugin || !isolate)
        {
            zend_string_release(real_path);
            goto DISPATCH;
        }
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            auto path = openrasp::NewV8String(isolate, Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename));
            params->Set(openrasp::NewV8String(isolate, "path"), path);
            params->Set(openrasp::NewV8String(isolate, "url"), path);
            params->Set(openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, real_path->val, real_path->len));
            zend_string_release(real_path);
            switch (opline->extended_value)
            {
            case ZEND_INCLUDE:
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "include"));
                break;
            case ZEND_INCLUDE_ONCE:
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "include_once"));
                break;
            case ZEND_REQUIRE:
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "require"));
                break;
            case ZEND_REQUIRE_ONCE:
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "require_once"));
                break;
            default:
                params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, ""));
                break;
            }
            is_block = isolate->Check(openrasp::NewV8String(isolate, "include"), params, openrasp_ini.timeout_ms);
        }
        if (is_block)
        {
            handle_block();
        }
    }

DISPATCH:
    if (real_path)
    {
        zend_string_release(real_path);
    }
    if (Z_TYPE(tmp_inc_filename) != IS_NULL)
    {
        zval_ptr_dtor(&tmp_inc_filename);
    }
    return ZEND_USER_OPCODE_DISPATCH;
}