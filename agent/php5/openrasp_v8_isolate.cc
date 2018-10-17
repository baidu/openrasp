#include "openrasp_v8.h"

namespace openrasp
{
Isolate *Isolate::New(Snapshot *snapshot_blob)
{
    Isolate::Data *data = new Isolate::Data();
    data->create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    data->create_params.snapshot_blob = snapshot_blob;
    data->create_params.external_references = snapshot_blob->external_references;

    v8::Isolate *isolate = v8::Isolate::New(data->create_params);
    isolate->Enter();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    context->Enter();

    auto key_action = NewV8String(isolate, "action");
    auto key_message = NewV8String(isolate, "message");
    auto key_name = NewV8String(isolate, "name");
    auto key_confidence = NewV8String(isolate, "confidence");
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
    auto JSON_stringify = context->Global()
                              ->Get(NewV8String(isolate, "JSON"))
                              .As<v8::Object>()
                              ->Get(NewV8String(isolate, "stringify"))
                              .As<v8::Function>();

    data->key_action.Reset(isolate, key_action);
    data->key_message.Reset(isolate, key_message);
    data->key_name.Reset(isolate, key_name);
    data->key_confidence.Reset(isolate, key_confidence);
    data->RASP.Reset(isolate, RASP);
    data->check.Reset(isolate, check);
    data->console_log.Reset(isolate, console_log);
    data->JSON_stringify.Reset(isolate, JSON_stringify);
    data->action_hash_ignore = NewV8String(isolate, "ignore")->GetIdentityHash();
    data->action_hash_log = NewV8String(isolate, "log")->GetIdentityHash();
    data->action_hash_block = NewV8String(isolate, "block")->GetIdentityHash();
    data->request_context.Reset(isolate, RequestContext::New(isolate));

    isolate->SetData(0, data);

    return reinterpret_cast<Isolate *>(isolate);
}

Isolate::Data *Isolate::GetData()
{
    return reinterpret_cast<Isolate::Data *>(v8::Isolate::GetData(0));
}

void Isolate::Dispose()
{
    Isolate::Data *data = GetData();
    delete data->create_params.array_buffer_allocator;
    delete data;
    {
        v8::HandleScope handle_scope(this);
        v8::Local<v8::Context> context = GetCurrentContext();
        context->Exit();
    }
    Exit();
    v8::Isolate::Dispose();
}
}