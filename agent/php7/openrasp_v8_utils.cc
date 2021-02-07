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
void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result);
void get_stack(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info);
CheckResult Check(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout)
{
    auto context = isolate->GetCurrentContext();
    auto data = isolate->GetData();
    params->SetLazyDataProperty(context, NewV8String(isolate, "stack", 5), get_stack).FromJust();
    v8::Local<v8::Object> request_context;
    if (data->request_context.IsEmpty())
    {
        request_context = data->request_context_templ.Get(isolate)->NewInstance(context).ToLocalChecked();
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
        auto action = obj->Get(context, NewV8String(isolate, "action")).FromMaybe(v8::Local<v8::Value>());
        if (action.IsEmpty() || !action->IsString())
        {
            continue;
        }
        std::string str = *v8::String::Utf8Value(isolate, action);
        if (str == "exception")
        {
            auto message = obj->Get(context, NewV8String(isolate, "message")).FromMaybe(v8::Local<v8::Value>());
            if (!message.IsEmpty() && message->IsString())
            {
                plugin_log(std::string(*v8::String::Utf8Value(isolate, message)) + "\n");
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
    auto context = isolate->GetCurrentContext();
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
                    arr->Set(context, index++, v8_value).IsJust();
                }
                else
                {
                    is_assoc = true;
                    rst = obj = v8::Object::New(isolate);
                    for (int i = 0; i < index; i++)
                    {
                        obj->Set(context, i, arr->Get(context, i).ToLocalChecked()).IsJust();
                    }
                }
            }
            if (is_assoc)
            {
                if (!key)
                {
                    obj->Set(context, idx, v8_value).IsJust();
                }
                else
                {
                    obj->Set(context, NewV8String(isolate, key->val, key->len), v8_value).IsJust();
                }
            }
        }
        ZEND_HASH_FOREACH_END();
        break;
    }
    case IS_STRING:
    {
        rst = NewV8String(isolate, Z_STRVAL_P(val), Z_STRLEN_P(val));
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

void plugin_log(const std::string &message)
{
    LOG_G(plugin_logger).log(LEVEL_INFO, message.c_str(), message.length(), false, true);
}

void get_stack(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::HandleScope handle_scope(isolate);
    auto arr = format_debug_backtrace_arr();
    size_t len = arr.size();
    auto stack = v8::Array::New(isolate, len);
    for (size_t i = 0; i < len; i++)
    {
        stack->Set(context, i, openrasp::NewV8String(isolate, arr[i])).IsJust();
    }
    info.GetReturnValue().Set(stack);
}

void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result)
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto undefined = v8::Undefined(isolate).As<v8::Value>();
    result->Set(context, NewV8String(isolate, "attack_type"), type).IsJust();
    result->Set(context, NewV8String(isolate, "intercept_state"), result->Get(context, NewV8String(isolate, "action")).FromMaybe(undefined)).IsJust();
    result->Set(context, NewV8String(isolate, "plugin_message"), result->Get(context, NewV8String(isolate, "message")).FromMaybe(undefined)).IsJust();
    result->Set(context, NewV8String(isolate, "plugin_confidence"), result->Get(context, NewV8String(isolate, "confidence")).FromMaybe(undefined)).IsJust();
    result->Set(context, NewV8String(isolate, "plugin_algorithm"), result->Get(context, NewV8String(isolate, "algorithm")).FromMaybe(undefined)).IsJust();
    result->Set(context, NewV8String(isolate, "plugin_name"), result->Get(context, NewV8String(isolate, "name")).FromMaybe(undefined)).IsJust();
    if (result->Has(context, NewV8String(isolate, "params")).FromMaybe(false))
    {
        result->Set(context, NewV8String(isolate, "attack_params"), result->Get(context, NewV8String(isolate, "params")).FromMaybe(undefined)).IsJust();
    }
    else
    {
        result->Set(context, NewV8String(isolate, "attack_params"), params).IsJust();
    }
    result->Delete(context, NewV8String(isolate, "action")).IsJust();
    result->Delete(context, NewV8String(isolate, "message")).IsJust();
    result->Delete(context, NewV8String(isolate, "confidence")).IsJust();
    result->Delete(context, NewV8String(isolate, "algorithm")).IsJust();
    result->Delete(context, NewV8String(isolate, "name")).IsJust();
    result->Delete(context, NewV8String(isolate, "params")).IsJust();

    v8::Local<v8::Value> val;
    if (v8::JSON::Stringify(isolate->GetCurrentContext(), result).ToLocal(&val))
    {
        v8::String::Utf8Value msg(isolate, val);
        openrasp::JsonReader base_json(*msg);
        if (!base_json.has_error())
        {
            LOG_G(alarm_logger).log(LEVEL_INFO, base_json);
        }
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
        v8::String::Utf8Value key(isolate, item.As<v8::Array>()->Get(context, 0).ToLocalChecked());
        v8::String::Utf8Value value(isolate, item.As<v8::Array>()->Get(context, 1).ToLocalChecked());
        auto iter = buildin_action_map.find({*key, key.length()});
        if (iter != buildin_action_map.end())
        {
            iter->second = std::string(*value, value.length());
        }
    }
}

std::vector<int64_t> extract_int64_array(Isolate *isolate, const std::string &value, int limit, const std::vector<int64_t> &default_value)
{
    if (nullptr != isolate)
    {
        std::string script = R"(
        (function () 
        {
            try {
                    return )" +
                             value + R"(.filter(value => typeof value === 'number');
                } catch (_) {}
        })()
        )";
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        auto rst = isolate->ExecScript(script, "extract_int64_array_" + value);
        if (!rst.IsEmpty())
        {
            std::vector<int64_t> result;
            auto v8_arr = rst.ToLocalChecked();
            if (!v8_arr.IsEmpty() && v8_arr->IsArray())
            {
                auto arr = v8_arr.As<v8::Array>();
                auto len = arr->Length();
                if (len > limit)
                {
                    openrasp_error(LEVEL_WARNING, PLUGIN_ERROR,
                                   _("Size of %s must <= %d."), value.c_str(), limit);
                }
                for (size_t i = 0; i < len; i++)
                {
                    v8::HandleScope handle_scope(isolate);
                    auto item = arr->Get(context, i).ToLocalChecked();
                    if (!item.IsEmpty() && item->IsInt32())
                    {
                        v8::Local<v8::Integer> err_code_local = item.As<v8::Integer>();
                        int64_t err_code = err_code_local->Value();
                        result.push_back(err_code);
                    }
                }
                return result;
            }
        }
    }
    return default_value;
}

std::vector<std::string> extract_string_array(Isolate *isolate, const std::string &value, int limit, const std::vector<std::string> &default_value)
{
    if (nullptr != isolate)
    {
        std::string script;
        script.append(R"( 
        (function () 
        {
            try {
                return )")
            .append(value)
            .append(R"(.filter(value => typeof value === 'string')
            } catch (_) {}
        })()
        )");
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        auto rst = isolate->ExecScript(script, "extract_string_array_" + value);
        if (!rst.IsEmpty())
        {
            std::vector<std::string> result;
            auto v8_arr = rst.ToLocalChecked();
            if (!v8_arr.IsEmpty() && v8_arr->IsArray())
            {
                auto arr = v8_arr.As<v8::Array>();
                auto len = arr->Length();
                if (len > limit)
                {
                    openrasp_error(LEVEL_WARNING, PLUGIN_ERROR,
                                   _("Size of %s must <= %d."), value.c_str(), limit);
                }
                for (size_t i = 0; i < len; i++)
                {
                    v8::HandleScope handle_scope(isolate);
                    v8::Local<v8::Value> item;
                    if (!arr->Get(context, i).ToLocal(&item) || !item->IsString())
                    {
                        continue;
                    }
                    v8::String::Utf8Value value(isolate, item);
                    result.push_back(std::string(*value, value.length()));
                }
                return result;
            }
        }
    }
    return default_value;
}

int64_t extract_int64(Isolate *isolate, const std::string &value, const int64_t &default_value)
{
    if (nullptr != isolate)
    {
        std::string script = R"(
        (function () 
        {
            try {
                    if (Number.isInteger()" +
                             value + R"())
                    {
                        return )" +
                             value + R"(
                    }
                } catch (_) {}
        })()
        )";
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        auto rst = isolate->ExecScript(script, "extract_int64_" + value);
        if (!rst.IsEmpty())
        {
            v8::HandleScope handle_scope(isolate);
            auto v8_value = rst.ToLocalChecked();
            if (!v8_value.IsEmpty() && v8_value->IsInt32())
            {
                v8::Local<v8::Integer> v8_result = v8_value.As<v8::Integer>();
                return v8_result->Value();
            }
        }
    }
    return default_value;
}

std::string extract_string(Isolate *isolate, const std::string &value, const std::string &default_value)
{
    if (nullptr != isolate)
    {
        std::string script = R"(
        (function () 
        {
            try {
                    if (typeof )" +
                             value + R"( === 'string')
                    {
                        return )" +
                             value + R"(
                    }
                } catch (_) {}
        })()
        )";
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        auto rst = isolate->ExecScript(script, "extract_string_" + value);
        if (!rst.IsEmpty())
        {
            v8::HandleScope handle_scope(isolate);
            auto v8_value = rst.ToLocalChecked();
            if (!v8_value.IsEmpty() && v8_value->IsString())
            {
                v8::Local<v8::String> v8_result = v8_value.As<v8::String>();
                v8::String::Utf8Value value(isolate, v8_result);
                return std::string(*value, value.length());
            }
        }
    }
    return default_value;
}

} // namespace openrasp