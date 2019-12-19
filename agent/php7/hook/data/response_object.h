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

#pragma once

#include "php_openrasp.h"
#include "openrasp_v8.h"
#include "policy_material.h"

namespace openrasp
{
namespace data
{

class ResponseObject : public PolicyMaterial
{
private:
    openrasp_v8::Isolate *isolate;
    const char *content;
    size_t length;
    const char *type;
    int timeout;

public:
    ResponseObject(openrasp_v8::Isolate *isolate, const char *content, size_t length, const char *type, int timeout) : isolate(isolate), content(content), length(length), type(type), timeout(timeout) {}
    virtual bool is_valid() const { return true; };
    virtual ulong hash() const { return 0; };
    virtual void fill_json_with_params(JsonReader &j) const {};
    virtual bool policy_check(JsonReader &j) const
    {
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        auto params = v8::Object::New(isolate);
        params->Set(context, openrasp_v8::NewV8String(isolate, "content"), openrasp_v8::NewV8String(isolate, content, length)).IsJust();
        params->Set(context, openrasp_v8::NewV8String(isolate, "content_type"), openrasp_v8::NewV8String(isolate, type)).IsJust();
        params->Set(context, openrasp_v8::NewV8String(isolate, "stack"), v8::Array::New(isolate)).IsJust();
        auto data = isolate->GetData();
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
        auto rst = isolate->Check(openrasp_v8::NewV8String(isolate, "response"), params, request_context, timeout);
        auto len = rst->Length();
        bool ret = false;
        for (int i = 0; i < len; i++)
        {
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Value> val;
            if (!rst->Get(context, i).ToLocal(&val) || !val->IsObject())
            {
                continue;
            }
            auto obj = val.As<v8::Object>();
            if (!obj->Get(context, NewV8String(isolate, "message")).ToLocal(&val) || !val->IsString())
            {
                continue;
            }
            ret = true;
            j.write_string({"policy_id"}, "3009");
            j.write_string({"message"}, *v8::String::Utf8Value(isolate, val));
            v8::Local<v8::String> str;
            if (obj->Get(context, NewV8String(isolate, "policy_params")).ToLocal(&val) &&
                val->IsObject() &&
                v8::JSON::Stringify(context, val).ToLocal(&str))
            {
                j.write_json_string({"policy_params"}, *v8::String::Utf8Value(isolate, str));
            }
            else
            {
                j.write_map({"policy_params"}, std::map<std::string, std::string>());
            }
        }
        return ret;
    };
};

} // namespace data

} // namespace openrasp