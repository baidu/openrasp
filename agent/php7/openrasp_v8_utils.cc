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

extern "C"
{
#include "php_scandir.h"
}
#include "openrasp_v8.h"
#include "openrasp_utils.h"
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include <iostream>
#include <sstream>

namespace openrasp
{
CheckResult Check(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout)
{
    IsolateData *data = isolate->GetData();
    v8::Local<v8::Object> request_context;
    if (data->request_context.IsEmpty())
    {
        request_context = data->request_context_templ.Get(isolate)->NewInstance();
        data->request_context.Reset(isolate, request_context);
    }
    else
    {
        request_context = data->request_context.Get(isolate);
    }
    auto rst = isolate->Check(type, params, request_context, timeout);
    auto len = rst->Length();
    if (len == 0)
    {
        return CheckResult::kCache;
    }
    auto context = isolate->GetCurrentContext();
    auto key_action = isolate->GetData()->key_action.Get(isolate);
    auto key_message = isolate->GetData()->key_message.Get(isolate);
    CheckResult check_result = CheckResult::kNoCache;
    for (int i = 0; i < len; i++)
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Value> val;
        if (!rst->Get(context, i).ToLocal(&val) || !val->IsObject())
        {
            continue;
        }
        auto obj = val.As<v8::Object>();
        auto action = obj->Get(context, key_action).FromMaybe(v8::Local<v8::Value>());
        if (action.IsEmpty() || !action->IsString())
        {
            continue;
        }
        std::string str = *v8::String::Utf8Value(isolate, action);
        if (str == "exception")
        {
            auto message = obj->Get(context, key_message).FromMaybe(v8::Local<v8::Value>());
            if (!message.IsEmpty() && message->IsString())
            {
                plugin_info(isolate, std::string(*v8::String::Utf8Value(isolate, message)) + "\n");
            }
            continue;
        }
        if (str == "block")
        {
            check_result = CheckResult::kBlock;
        }
        alarm_info(isolate, type, params, obj);
    }
    return check_result;
}

v8::Local<v8::Value> NewV8ValueFromZval(v8::Isolate *isolate, zval *val)
{
    v8::Local<v8::Value> rst = v8::Undefined(isolate);
    switch (Z_TYPE_P(val))
    {
    case IS_ARRAY:
    {
        HashTable *ht = Z_ARRVAL_P(val);
        if (!ht)
        {
            break;
        }

        v8::Local<v8::Array> arr;
        v8::Local<v8::Object> obj;
        rst = arr = v8::Array::New(isolate);
        bool is_assoc = false;
        size_t index = 0;

        zval *value;
        zend_string *key;
        zend_ulong idx;
        ZEND_HASH_FOREACH_KEY_VAL(ht, idx, key, value)
        {
            v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, value);
            if (!is_assoc)
            {
                if (index == idx)
                {
                    arr->Set(index++, v8_value);
                }
                else
                {
                    is_assoc = true;
                    rst = obj = v8::Object::New(isolate);
                    for (int i = 0; i < index; i++)
                    {
                        obj->Set(i, arr->Get(i));
                    }
                }
            }
            if (is_assoc)
            {
                if (!key)
                {
                    obj->Set(idx, v8_value);
                }
                else
                {
                    obj->Set(NewV8String(isolate, key->val, key->len), v8_value);
                }
            }
        }
        ZEND_HASH_FOREACH_END();
        break;
    }
    case IS_STRING:
    {
        bool avoidwarning = v8::String::NewFromUtf8(isolate, Z_STRVAL_P(val), v8::NewStringType::kNormal, Z_STRLEN_P(val)).ToLocal(&rst);
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
    case IS_TRUE:
        rst = v8::Boolean::New(isolate, true);
        break;
    case IS_FALSE:
        rst = v8::Boolean::New(isolate, false);
        break;
    default:
        rst = v8::Undefined(isolate);
        break;
    }
    return rst;
}

void plugin_info(Isolate *isolate, const std::string &message)
{
    LOG_G(plugin_logger).log(LEVEL_INFO, message.c_str(), message.length(), false, true);
}

void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result)
{
    auto key_action = isolate->GetData()->key_action.Get(isolate);
    auto key_message = isolate->GetData()->key_message.Get(isolate);
    auto key_confidence = isolate->GetData()->key_confidence.Get(isolate);
    auto key_algorithm = isolate->GetData()->key_algorithm.Get(isolate);
    auto key_name = isolate->GetData()->key_name.Get(isolate);

    auto stack_trace = NewV8String(isolate, format_debug_backtrace_str());

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
    obj->Set(NewV8String(isolate, "plugin_algorithm"), result->Get(key_algorithm));
    obj->Set(NewV8String(isolate, "plugin_name"), result->Get(key_name));
    obj->Set(NewV8String(isolate, "stack_trace"), stack_trace);
    obj->Set(NewV8String(isolate, "event_time"), event_time);
    {
        auto source_code = v8::Array::New(isolate);
        if (OPENRASP_CONFIG(decompile.enable))
        {
            auto src = format_source_code_arr(TSRMLS_C);
            size_t len = src.size();
            source_code = v8::Array::New(isolate, len);
            for (size_t i = 0; i < len; i++)
            {
                source_code->Set(i, openrasp::NewV8String(isolate, src[i]));
            }
        }
        obj->Set(NewV8String(isolate, "source_code"), source_code);
    }

    zval *value;
    zend_string *key;
    zval *alarm_common_info = LOG_G(alarm_logger).get_common_info();
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(alarm_common_info), key, value)
    {
        if (key == nullptr ||
            (Z_TYPE_P(value) != IS_STRING &&
             Z_TYPE_P(value) != IS_LONG &&
             Z_TYPE_P(value) != IS_ARRAY))
        {
            continue;
        }
        obj->Set(NewV8String(isolate, key->val, key->len), NewV8ValueFromZval(isolate, value));
    }
    ZEND_HASH_FOREACH_END();

    v8::Local<v8::Value> val;
    if (v8::JSON::Stringify(isolate->GetCurrentContext(), obj).ToLocal(&val))
    {
        v8::String::Utf8Value msg(isolate, val);
        LOG_G(alarm_logger).log(LEVEL_INFO, *msg, msg.length(), true, false);
    }
}

void load_plugins()
{
    std::vector<PluginFile> plugin_src_list;
    std::string plugin_path(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("plugins"));
    dirent **ent = nullptr;
    int n_plugin = php_scandir(plugin_path.c_str(), &ent, nullptr, php_alphasort);
    for (int i = 0; i < n_plugin; i++)
    {
        const char *p = strrchr(ent[i]->d_name, '.');
        if (p != nullptr && strcasecmp(p, ".js") == 0)
        {
            std::string filename(ent[i]->d_name);
            std::string filepath(plugin_path + DEFAULT_SLASH + filename);
            struct stat sb;
            if (VCWD_STAT(filepath.c_str(), &sb) == 0 && (sb.st_mode & S_IFREG) != 0)
            {
                std::ifstream file(filepath);
                std::streampos beg = file.tellg();
                file.seekg(0, std::ios::end);
                std::streampos end = file.tellg();
                file.seekg(0, std::ios::beg);
                // plugin file size limitation: 10 MB
                if (10 * 1024 * 1024 >= end - beg)
                {
                    std::string source((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                    plugin_src_list.emplace_back(filename, source);
                }
                else
                {
                    openrasp_error(LEVEL_WARNING, PLUGIN_ERROR, _("Ignored Javascript plugin file '%s', as it exceeds 10 MB in file size."), filename.c_str());
                }
            }
        }
        free(ent[i]);
    }
    free(ent);
    process_globals.plugin_src_list = plugin_src_list;
}

void extract_buildin_action(Isolate *isolate, std::map<std::string, std::string> &buildin_action_map)
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    // clang-format off
    auto rst = isolate->ExecScript(R"(
        Object.keys(RASP.algorithmConfig || {})
            .filter(key => typeof key === 'string' && typeof RASP.algorithmConfig[key] === 'object' && typeof RASP.algorithmConfig[key].action === 'string')
            .map(key => [key, RASP.algorithmConfig[key].action])
    )", "extract_buildin_action");
    // clang-format on
    if (rst.IsEmpty())
    {
        return;
    }
    auto arr = rst.ToLocalChecked().As<v8::Array>();
    auto len = arr->Length();
    for (size_t i = 0; i < len; i++)
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Value> item;
        if (!arr->Get(context, i).ToLocal(&item) || !item->IsArray())
        {
            continue;
        }
        v8::String::Utf8Value key(isolate, item.As<v8::Array>()->Get(0));
        v8::String::Utf8Value value(isolate, item.As<v8::Array>()->Get(1));
        auto iter = buildin_action_map.find({*key, key.length()});
        if (iter != buildin_action_map.end())
        {
            iter->second = std::string(*value, value.length());
        }
    }
}

} // namespace openrasp