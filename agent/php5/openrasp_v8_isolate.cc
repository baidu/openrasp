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

#include "openrasp_v8_bundle.h"

namespace openrasp
{
Isolate *Isolate::New(Snapshot *snapshot_blob)
{
    Isolate::Data *data = new Isolate::Data();
    data->create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    data->create_params.snapshot_blob = snapshot_blob;
    data->create_params.external_references = snapshot_blob->external_references;

    Isolate *isolate = reinterpret_cast<Isolate *>(v8::Isolate::New(data->create_params));
    isolate->Enter();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    context->Enter();

    auto key_action = NewV8String(isolate, "action");
    auto key_message = NewV8String(isolate, "message");
    auto key_name = NewV8String(isolate, "name");
    auto key_confidence = NewV8String(isolate, "confidence");
    auto key_algorithm = NewV8String(isolate, "algorithm");
    auto RASP = context->Global()
                    ->Get(context, NewV8String(isolate, "RASP"))
                    .ToLocalChecked()
                    .As<v8::Object>();
    auto check = RASP->Get(context, NewV8String(isolate, "check"))
                     .ToLocalChecked()
                     .As<v8::Function>();
    auto console_log = context->Global()
                           ->Get(NewV8String(isolate, "console"))
                           .As<v8::Object>()
                           ->Get(NewV8String(isolate, "log"))
                           .As<v8::Function>();

    data->key_action.Reset(isolate, key_action);
    data->key_message.Reset(isolate, key_message);
    data->key_name.Reset(isolate, key_name);
    data->key_confidence.Reset(isolate, key_confidence);
    data->key_algorithm.Reset(isolate, key_algorithm);
    data->RASP.Reset(isolate, RASP);
    data->check.Reset(isolate, check);
    data->console_log.Reset(isolate, console_log);
    data->action_hash_ignore = NewV8String(isolate, "ignore")->GetIdentityHash();
    data->action_hash_log = NewV8String(isolate, "log")->GetIdentityHash();
    data->action_hash_block = NewV8String(isolate, "block")->GetIdentityHash();
    data->request_context_templ.Reset(isolate, NewRequestContextTemplate(isolate));

    isolate->v8::Isolate::SetData(0, data);

    return isolate;
}

Isolate::Data *Isolate::GetData()
{
    return reinterpret_cast<Isolate::Data *>(v8::Isolate::GetData(0));
}

void Isolate::Dispose()
{
    Isolate::Data *data = GetData();
    {
        v8::HandleScope handle_scope(this);
        v8::Local<v8::Context> context = GetCurrentContext();
        context->Exit();
    }
    Exit();
    v8::Isolate::Dispose();
    delete data->create_params.array_buffer_allocator;
    delete data;
}

bool Isolate::IsExpired(uint64_t timestamp)
{
    return timestamp > GetData()->timestamp;
}

bool Isolate::Check(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout)
{
    Isolate::Data *data = isolate->GetData();
    auto context = isolate->GetCurrentContext();
    v8::TryCatch try_catch;
    auto check = data->check.Get(isolate);
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
    v8::Local<v8::Value> argv[]{type, params, request_context};

    v8::Local<v8::Value> rst;
    auto task = new TimeoutTask(isolate, timeout);
    task->GetMtx().lock();
    Platform::platform->CallOnBackgroundThread(task, v8::Platform::kShortRunningTask);
    (void)check->Call(context, check, 3, argv).ToLocal(&rst);
    task->GetMtx().unlock();
    if (UNLIKELY(rst.IsEmpty()))
    {
        v8::Local<v8::Value> message = try_catch.StackTrace();
        if (message.IsEmpty())
        {
            auto msg = v8::Object::New(isolate);
            msg->Set(NewV8String(isolate, "message"), NewV8String(isolate, "Javascript plugin execution timeout."));
            msg->Set(NewV8String(isolate, "type"), type);
            msg->Set(NewV8String(isolate, "params"), params);
            msg->Set(NewV8String(isolate, "context"), request_context);
            message = msg;
        }
        plugin_info(isolate, message);
        return false;
    }
    if (UNLIKELY(!rst->IsArray()))
    {
        return false;
    }
    auto key_action = data->key_action.Get(isolate);

    auto arr = v8::Local<v8::Array>::Cast(rst);
    int len = arr->Length();
    bool is_block = false;
    for (int i = 0; i < len; i++)
    {
        auto item = v8::Local<v8::Object>::Cast(arr->Get(i));
        v8::Local<v8::Value> v8_action = item->Get(key_action);
        if (UNLIKELY(!v8_action->IsString()))
        {
            continue;
        }
        int action_hash = v8_action->ToString()->GetIdentityHash();
        if (LIKELY(data->action_hash_ignore == action_hash))
        {
            continue;
        }
        is_block = is_block || data->action_hash_block == action_hash;

        alarm_info(isolate, type, params, item);
    }
    return is_block;
}

bool Isolate::Check(v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout)
{
    return Check(this, type, params, timeout);
}

v8::MaybeLocal<v8::Value> Isolate::ExecScript(Isolate *isolate, std::string _source, std::string _filename, int _line_offset)
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::String> filename = NewV8String(isolate, _filename);
    v8::Local<v8::Integer> line_offset = v8::Integer::New(isolate, _line_offset);
    v8::Local<v8::String> source = NewV8String(isolate, _source);
    v8::ScriptOrigin origin(filename, line_offset);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source, &origin).ToLocal(&script))
    {
        return {};
    }
    return script->Run(context);
}

v8::MaybeLocal<v8::Value> Isolate::ExecScript(std::string _source, std::string _filename, int _line_offset)
{
    return ExecScript(this, _source, _filename, _line_offset);
}

void Isolate::extract_buildin_action(Isolate *isolate, std::map<std::string, std::string> &buildin_action_map)
{
    auto context = isolate->GetCurrentContext();
    auto RASP = context->Global()
                    ->Get(context, NewV8String(isolate, "RASP"))
                    .ToLocalChecked()
                    .As<v8::Object>();
    auto algorithm_config_ml = RASP->Get(context, NewV8String(isolate, "algorithmConfig"));
    if (algorithm_config_ml.IsEmpty())
    {
        return;
    }
    auto algorithmConfig_l = algorithm_config_ml.ToLocalChecked();
    if (!algorithmConfig_l->IsObject())
    {
        return;
    }
    auto algorithmConfig = algorithmConfig_l.As<v8::Object>();
    auto props_ml = algorithmConfig->GetOwnPropertyNames(context);
    if (props_ml.IsEmpty())
    {
        return;
    }
    auto props = props_ml.ToLocalChecked();
    for (int i = 0, l = props->Length(); i < l; i++)
    {
        auto localKey = props->Get(i);
        if (!localKey->IsString())
        {
            continue;
        }
        std::string key = *v8::String::Utf8Value(localKey);
        auto iter = buildin_action_map.find(key);
        if (iter != buildin_action_map.end())
        {
            auto action_obj_ml = algorithmConfig->Get(context, localKey);
            if (action_obj_ml.IsEmpty())
            {
                continue;
            }
            auto action_obj_l = action_obj_ml.ToLocalChecked();
            if (!action_obj_l->IsObject())
            {
                continue;
            }
            auto action_obj = action_obj_l.As<v8::Object>();
            auto action_ml = action_obj->Get(context, NewV8String(isolate, "action"));
            if (action_ml.IsEmpty())
            {
                continue;
            }
            auto action_l = action_ml.ToLocalChecked();
            if (action_l->IsString())
            {
                std::string val = *v8::String::Utf8Value(action_l);
                buildin_action_map[key] = val;
            }
        }
    }
}
} // namespace openrasp