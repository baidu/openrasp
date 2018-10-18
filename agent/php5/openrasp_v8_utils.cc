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

#include "openrasp_v8.h"
#include "openrasp_utils.h"
#include "openrasp_log.h"
#include <iostream>
#include <sstream>

namespace openrasp
{
void v8error_to_stream(v8::Isolate *isolate, v8::TryCatch &try_catch, std::ostream &buf)
{
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(try_catch.Exception());
    const char *exception_string = *exception;
    v8::Local<v8::Message> message = try_catch.Message();
    if (message.IsEmpty())
    {
        buf << exception_string << "\n";
    }
    else
    {
        v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char *filename_string = *filename;
        int linenum = message->GetLineNumber(context).FromJust();
        buf << filename_string << ":" << linenum << "\n";
        v8::String::Utf8Value sourceline(
            message->GetSourceLine(context).ToLocalChecked());
        const char *sourceline_string = *sourceline;
        buf << sourceline_string << "\n";
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++)
        {
            buf << " ";
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++)
        {
            buf << "^";
        }
        buf << "\n";
        v8::Local<v8::Value> stack_trace_string;
        if (try_catch.StackTrace(context).ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString() &&
            v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0)
        {
            v8::String::Utf8Value stack_trace(stack_trace_string);
            const char *stack_trace_string = *stack_trace;
            buf << stack_trace_string << "\n";
        }
    }
}

v8::Local<v8::Value> zval_to_v8val(zval *val, v8::Isolate *isolate TSRMLS_DC)
{
    v8::Local<v8::Value> rst = v8::Undefined(isolate);
    switch (Z_TYPE_P(val))
    {
    case IS_ARRAY:
    {
        HashTable *ht = Z_ARRVAL_P(val);
        if (!ht || ht->nApplyCount > 1)
        {
            rst = v8::Undefined(isolate);
            break;
        }
        int num = zend_hash_num_elements(ht);
        v8::Local<v8::Object> obj;
        v8::Local<v8::Array> arr = v8::Array::New(isolate, num);
        rst = arr;
        if (num == 0)
        {
            break;
        }
        bool is_assoc = false;
        int index = 0;
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(val)); zend_hash_has_more_elements(Z_ARRVAL_P(val)) == SUCCESS; zend_hash_move_forward(Z_ARRVAL_P(val)))
        {
            char *key;
            ulong idx;
            int type;
            zval **value;
            if (zend_hash_get_current_data(Z_ARRVAL_P(val), (void **)&value) != SUCCESS)
            {
                continue;
            }
            HashTable *ht = HASH_OF(*value);
            if (ht)
            {
                ht->nApplyCount++;
            }
            v8::Local<v8::Value> v8_value = zval_to_v8val(*value, isolate TSRMLS_CC);
            if (ht)
            {
                ht->nApplyCount--;
            }
            type = zend_hash_get_current_key(Z_ARRVAL_P(val), &key, &idx, 0);
            if (!is_assoc)
            {
                if (type == HASH_KEY_IS_LONG && index == idx)
                {
                    arr->Set(index++, v8_value);
                }
                else
                {
                    is_assoc = true;
                    obj = v8::Object::New(isolate);
                    rst = obj;
                    for (int i = 0; i < index; i++)
                    {
                        obj->Set(i, arr->Get(i));
                    }
                }
            }
            if (is_assoc)
            {
                if (type == HASH_KEY_IS_LONG)
                {
                    obj->Set(idx, v8_value);
                }
                else
                {
                    v8::Local<v8::String> v8_key;
                    if (V8STRING_I(key).ToLocal(&v8_key))
                    {
                        obj->Set(v8_key, v8_value);
                    }
                }
            }
        }
        break;
    }
    case IS_STRING:
    {
        bool avoidwarning = v8::String::NewFromOneByte(isolate, (uint8_t *)Z_STRVAL_P(val), v8::NewStringType::kNormal, Z_STRLEN_P(val)).ToLocal(&rst);
        break;
    }
    case IS_LONG:
    {
        int64_t v = Z_LVAL_P(val);
        if (v < std::numeric_limits<int32_t>::min() || v > std::numeric_limits<int32_t>::max())
        {
            rst = v8::Number::New(isolate, v);
        }
        else
        {
            rst = v8::Int32::New(isolate, v);
        }
        break;
    }
    case IS_DOUBLE:
        rst = v8::Number::New(isolate, Z_DVAL_P(val));
        break;
    case IS_BOOL:
        rst = v8::Boolean::New(isolate, Z_BVAL_P(val));
        break;
    default:
        rst = v8::Undefined(isolate);
        break;
    }
    return rst;
}

v8::MaybeLocal<v8::Script> compile_script(std::string _source, std::string _filename, int _line_offset)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> filename;
    if (!V8STRING_EX(_filename.c_str(), v8::NewStringType::kNormal, _filename.length()).ToLocal(&filename))
    {
        return v8::MaybeLocal<v8::Script>();
    }
    v8::Local<v8::String> source;
    if (!V8STRING_EX(_source.c_str(), v8::NewStringType::kNormal, _source.length()).ToLocal(&source))
    {
        return v8::MaybeLocal<v8::Script>();
    }
    v8::Local<v8::Integer> line_offset = v8::Integer::New(isolate, _line_offset);
    v8::ScriptOrigin origin(filename, line_offset);
    v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source, &origin);
    return script;
}

v8::MaybeLocal<v8::Value> exec_script(v8::Isolate *isolate, v8::Local<v8::Context> context,
                                      std::string _source, std::string _filename, int _line_offset)
{
    v8::MaybeLocal<v8::Value> result;
    v8::Local<v8::String> filename;
    if (!V8STRING_EX(_filename.c_str(), v8::NewStringType::kNormal, _filename.length()).ToLocal(&filename))
    {
        return result;
    }
    v8::Local<v8::String> source;
    if (!V8STRING_EX(_source.c_str(), v8::NewStringType::kNormal, _source.length()).ToLocal(&source))
    {
        return result;
    }
    v8::Local<v8::Integer> line_offset = v8::Integer::New(isolate, _line_offset);
    v8::ScriptOrigin origin(filename, line_offset);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source, &origin).ToLocal(&script))
    {
        return result;
    }
    return script->Run(context);
}

void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result)
{
    TSRMLS_FETCH();
    auto JSON_stringify = isolate->GetData()->JSON_stringify.Get(isolate);
    auto key_action = isolate->GetData()->key_action.Get(isolate);
    auto key_message = isolate->GetData()->key_message.Get(isolate);
    auto key_confidence = isolate->GetData()->key_confidence.Get(isolate);
    auto key_name = isolate->GetData()->key_name.Get(isolate);

    auto stack_trace = NewV8String(isolate, format_debug_backtrace_str(TSRMLS_C));

    std::time_t t = std::time(nullptr);
    char buffer[100] = {0};
    size_t size = std::strftime(buffer, sizeof(buffer), RaspLoggerEntry::rasp_rfc3339_format, std::localtime(&t));
    auto event_time = NewV8String(isolate, buffer, size);

    auto obj = v8::Object::New(isolate);
    obj->Set(NewV8String(isolate, "attack_type"), type);
    obj->Set(NewV8String(isolate, "attack_params"), params);
    obj->Set(NewV8String(isolate, "intercept_state"), result->Get(key_action));
    obj->Set(NewV8String(isolate, "plugin_message"), result->Get(key_message));
    obj->Set(NewV8String(isolate, "plugin_confidence"), result->Get(key_confidence));
    obj->Set(NewV8String(isolate, "plugin_name"), result->Get(key_name));
    obj->Set(NewV8String(isolate, "stack_trace"), stack_trace);
    obj->Set(NewV8String(isolate, "event_time"), event_time);
    zval *alarm_common_info = LOG_G(alarm_logger).get_common_info(TSRMLS_C);
    HashTable *ht = Z_ARRVAL_P(alarm_common_info);
    for (zend_hash_internal_pointer_reset(ht);
         zend_hash_has_more_elements(ht) == SUCCESS;
         zend_hash_move_forward(ht))
    {
        char *key;
        ulong idx;
        int type;
        zval **value;
        type = zend_hash_get_current_key(ht, &key, &idx, 0);
        if (type != HASH_KEY_IS_STRING ||
            zend_hash_get_current_data(ht, (void **)&value) != SUCCESS ||
            Z_TYPE_PP(value) != IS_STRING)
        {
            continue;
        }
        obj->Set(NewV8String(isolate, key), NewV8String(isolate, Z_STRVAL_PP(value), Z_STRLEN_PP(value)));
    }
    v8::Local<v8::Value> val;
    if (JSON_stringify->Call(isolate->GetCurrentContext(), JSON_stringify, 1, reinterpret_cast<v8::Local<v8::Value> *>(&obj)).ToLocal(&val))
    {
        v8::String::Utf8Value msg(val);
        LOG_G(alarm_logger).log(LEVEL_INFO, *msg, msg.length() TSRMLS_CC, false);
    }
}

} // namespace openrasp