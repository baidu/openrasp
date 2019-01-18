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

namespace openrasp
{
v8::Local<v8::Value> NewV8ValueFromZval(v8::Isolate *isolate, zval *val)
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
        TSRMLS_FETCH();
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
            v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, *value);
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
                    obj->Set(NewV8String(isolate, key), v8_value);
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

void plugin_info(const char *message, size_t length)
{
    TSRMLS_FETCH();
    LOG_G(plugin_logger).log(LEVEL_INFO, message, length TSRMLS_CC, false, true);
}

void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result)
{
    TSRMLS_FETCH();
    auto key_action = isolate->GetData()->key_action.Get(isolate);
    auto key_message = isolate->GetData()->key_message.Get(isolate);
    auto key_confidence = isolate->GetData()->key_confidence.Get(isolate);
    auto key_algorithm = isolate->GetData()->key_algorithm.Get(isolate);
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
    obj->Set(NewV8String(isolate, "plugin_algorithm"), result->Get(key_algorithm));
    obj->Set(NewV8String(isolate, "plugin_name"), result->Get(key_name));
    obj->Set(NewV8String(isolate, "stack_trace"), stack_trace);
    obj->Set(NewV8String(isolate, "event_time"), event_time);
    if (OPENRASP_CONFIG(decompile.enable))
    {
        auto src = format_source_code_arr(TSRMLS_C);
        size_t len = src.size();
        auto source_code = v8::Array::New(isolate, len);
        for (size_t i = 0; i < len; i++)
        {
            source_code->Set(i, openrasp::NewV8String(isolate, src[i]));
        }
        obj->Set(NewV8String(isolate, "source_code"), source_code);
    }

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
            (Z_TYPE_PP(value) != IS_STRING &&
             Z_TYPE_PP(value) != IS_LONG &&
             Z_TYPE_PP(value) != IS_ARRAY))
        {
            continue;
        }
        obj->Set(NewV8String(isolate, key), NewV8ValueFromZval(isolate, *value));
    }
    v8::Local<v8::String> val;
    if (v8::JSON::Stringify(isolate->GetCurrentContext(), obj).ToLocal(&val))
    {
        v8::String::Utf8Value msg(val);
        LOG_G(alarm_logger).log(LEVEL_INFO, *msg, msg.length() TSRMLS_CC, true, false);
    }
}

void load_plugins()
{
    TSRMLS_FETCH();
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
    auto rst = isolate->ExecScript(R"(
        Object.keys(RASP.algorithmConfig || {})
            .filter(key => typeof key === 'string' && typeof RASP.algorithmConfig[key] === 'object' && typeof RASP.algorithmConfig[key].action === 'string')
            .map(key => [key, RASP.algorithmConfig[key].action])
    )",
                                   "extract_buildin_action");
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
        v8::String::Utf8Value key(item.As<v8::Array>()->Get(0));
        v8::String::Utf8Value value(item.As<v8::Array>()->Get(1));
        auto iter = buildin_action_map.find({*key, key.length()});
        if (iter != buildin_action_map.end())
        {
            iter->second = std::string(*value, value.length());
        }
    }
}

void extract_callable_blacklist(Isolate *isolate, std::vector<std::string> &callable_blacklist)
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto rst = isolate->ExecScript(R"(
        (function () {
            var blacklist
            try {
                blacklist = RASP.algorithmConfig.webshell_callable.functions
            } catch (_) {
            }
            if (blacklist === undefined || !Array.isArray(blacklist)) {
                blacklist = ["system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"]
            }
            return blacklist
        })()
    )",
                                   "extract_callable_blacklist");
    if (!rst.IsEmpty())
    {
        callable_blacklist.clear();
        auto arr = rst.ToLocalChecked().As<v8::Array>();
        auto len = arr->Length();
        for (size_t i = 0; i < len; i++)
        {
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Value> item;
            if (!arr->Get(context, i).ToLocal(&item) || !item->IsString())
            {
                continue;
            }
            v8::String::Utf8Value value(item);
            callable_blacklist.push_back(std::string(*value, value.length()));
        }
    }
}

void extract_xss_config(Isolate *isolate, std::string &filter_regex, int64_t &min_length, int64_t &max_detection_num)
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto rst = isolate->ExecScript(R"(
        (function () {
            var filter_regex = "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]"
            var min_length = 15
            var max_detection_num = 10
            try {
                var xss_userinput = RASP.algorithmConfig.xss_userinput
                if (typeof xss_userinput.filter_regex === 'string') {
                    filter_regex = xss_userinput.filter_regex
                }
                if (Number.isInteger(xss_userinput.min_length)) {
                    min_length = xss_userinput.min_length
                }
                if (Number.isInteger(xss_userinput.max_detection_num)) {
                    max_detection_num = xss_userinput.max_detection_num
                }
            } catch (_) {
            }
            return [filter_regex, min_length, max_detection_num]
        })()
    )",
                                   "extract_xss_config");
    if (!rst.IsEmpty())
    {

        auto arr = rst.ToLocalChecked().As<v8::Array>();
        auto len = arr->Length();
        if (3 == len)
        {
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Value> item0;
            if (arr->Get(context, 0).ToLocal(&item0) && item0->IsString())
            {
                v8::String::Utf8Value value(item0);
                filter_regex = std::string(*value, value.length());
            }
            v8::Local<v8::Value> item1;
            if (arr->Get(context, 1).ToLocal(&item1) && item1->IsNumber())
            {
                min_length = item1->IntegerValue();
            }
            v8::Local<v8::Value> item2;
            if (arr->Get(context, 2).ToLocal(&item2) && item2->IsNumber())
            {
                max_detection_num = item2->IntegerValue();
            }
        }
    }
}

} // namespace openrasp