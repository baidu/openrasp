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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_openrasp.h"
}
#include <sstream>
#include <fstream>
#include "openrasp_v8.h"
#include "openrasp_ini.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

namespace openrasp
{
openrasp_v8_process_globals process_globals;
} // namespace openrasp

using namespace openrasp;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8)
{
<<<<<<< HEAD
#ifdef ZTS
    new (openrasp_v8_globals) _zend_openrasp_v8_globals;
#endif
=======
    if (!init_isolate(TSRMLS_C))
    {
        return 0;
    }
    v8::Isolate *isolate = OPENRASP_V8_G(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handlescope(isolate);
    v8::Local<v8::Context> context = OPENRASP_V8_G(context).Get(isolate);
    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch;
    v8::Local<v8::Function> check = OPENRASP_V8_G(check).Get(isolate);
    v8::Local<v8::Value> type = V8STRING_N(c_type).ToLocalChecked();
    v8::Local<v8::Value> params = zval_to_v8val(z_params, isolate TSRMLS_CC);
    v8::Local<v8::Object> request_context = OPENRASP_V8_G(request_context).Get(isolate);
    v8::Local<v8::Value> argv[]{type, params, request_context};

    v8::Local<v8::Value> rst;
    {
        TimeoutTask *task = new TimeoutTask(isolate, openrasp_ini.timeout_ms);
        std::lock_guard<std::timed_mutex> lock(task->GetMtx());
        process_globals.v8_platform->CallOnBackgroundThread(task, v8::Platform::kShortRunningTask);
        bool avoidwarning = check->Call(context, check, 3, argv).ToLocal(&rst);
    }
    if (rst.IsEmpty())
    {
        if (try_catch.Message().IsEmpty())
        {
            v8::Local<v8::Function> console_log = context->Global()
                                                      ->Get(context, V8STRING_I("console").ToLocalChecked())
                                                      .ToLocalChecked()
                                                      .As<v8::Object>()
                                                      ->Get(context, V8STRING_I("log").ToLocalChecked())
                                                      .ToLocalChecked()
                                                      .As<v8::Function>();
            v8::Local<v8::Object> message = v8::Object::New(isolate);
            message->Set(V8STRING_N("message").ToLocalChecked(), V8STRING_N("Javascript plugin execution timeout.").ToLocalChecked());
            message->Set(V8STRING_N("type").ToLocalChecked(), type);
            message->Set(V8STRING_N("params").ToLocalChecked(), params);
            message->Set(V8STRING_N("context").ToLocalChecked(), request_context);
            bool avoidwarning = console_log->Call(context, console_log, 1, reinterpret_cast<v8::Local<v8::Value> *>(&message)).IsEmpty();
        }
        else
        {
            std::stringstream stream;
            v8error_to_stream(isolate, try_catch, stream);
            std::string error = stream.str();
            plugin_info(error.c_str(), error.length() TSRMLS_CC);
        }
        return 0;
    }
    if (!rst->IsArray())
    {
        return 0;
    }
    v8::Local<v8::String> key_action = OPENRASP_V8_G(key_action).Get(isolate);
    v8::Local<v8::String> key_message = OPENRASP_V8_G(key_message).Get(isolate);
    v8::Local<v8::String> key_name = OPENRASP_V8_G(key_name).Get(isolate);
    v8::Local<v8::String> key_confidence = OPENRASP_V8_G(key_confidence).Get(isolate);
    v8::Local<v8::String> key_algorithm = OPENRASP_V8_G(key_algorithm).Get(isolate);

    v8::Local<v8::Array> arr = rst.As<v8::Array>();
    int len = arr->Length();
    bool is_block = false;
    for (int i = 0; i < len; i++)
    {
        v8::Local<v8::Object> item = arr->Get(i).As<v8::Object>();
        v8::Local<v8::Value> v8_action = item->Get(key_action);
        if (!v8_action->IsString())
        {
            continue;
        }
        int action_hash = v8_action->ToString()->GetIdentityHash();
        if (OPENRASP_V8_G(action_hash_ignore) == action_hash)
        {
            continue;
        }
        is_block = is_block || OPENRASP_V8_G(action_hash_block) == action_hash;

        v8::Local<v8::Value> v8_message = item->Get(key_message);
        v8::Local<v8::Value> v8_name = item->Get(key_name);
        v8::Local<v8::Value> v8_confidence = item->Get(key_confidence);
        v8::Local<v8::Value> v8_algorithm = item->Get(key_algorithm);
        v8::String::Utf8Value utf_action(v8_action);
        v8::String::Utf8Value utf_message(v8_message);
        v8::String::Utf8Value utf_name(v8_name);
        v8::String::Utf8Value utf_algorithm(v8_algorithm);

        zval z_type, z_action, z_message, z_name, z_confidence, z_algorithm;
        INIT_ZVAL(z_type);
        INIT_ZVAL(z_action);
        INIT_ZVAL(z_message);
        INIT_ZVAL(z_name);
        INIT_ZVAL(z_confidence);
        INIT_ZVAL(z_algorithm);
        ZVAL_STRING(&z_type, c_type, 0);
        ZVAL_STRINGL(&z_action, *utf_action, utf_action.length(), 0);
        ZVAL_STRINGL(&z_message, *utf_message, utf_message.length(), 0);
        ZVAL_STRINGL(&z_name, *utf_name, utf_name.length(), 0);
        ZVAL_LONG(&z_confidence, v8_confidence->Int32Value());
        ZVAL_STRINGL(&z_algorithm, *utf_algorithm, utf_algorithm.length(), 0);

        zval result;
        INIT_ZVAL(result);
        ALLOC_HASHTABLE(Z_ARRVAL(result));
        // 设置 zend hash 的析构函数为空
        // 便于用共享 v8 产生的字符串，减少内存分配
        zend_hash_init(Z_ARRVAL(result), 0, 0, 0, 0);
        Z_TYPE(result) = IS_ARRAY;
        add_assoc_zval(&result, "attack_type", &z_type);
        add_assoc_zval(&result, "attack_params", z_params);
        add_assoc_zval(&result, "intercept_state", &z_action);
        add_assoc_zval(&result, "plugin_message", &z_message);
        add_assoc_zval(&result, "plugin_name", &z_name);
        add_assoc_zval(&result, "plugin_confidence", &z_confidence);
        add_assoc_zval(&result, "plugin_algorithm", &z_algorithm);
        alarm_info(&result TSRMLS_CC);
        zval_dtor(&result);
    }
    return is_block ? 1 : 0;
>>>>>>> master
}

PHP_GSHUTDOWN_FUNCTION(openrasp_v8)
{
    if (openrasp_v8_globals->isolate)
    {
        openrasp_v8_globals->isolate->Dispose();
        openrasp_v8_globals->isolate = nullptr;
    }
#ifdef ZTS
    openrasp_v8_globals->~_zend_openrasp_v8_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp_v8)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_v8, PHP_GINIT(openrasp_v8), PHP_GSHUTDOWN(openrasp_v8));

    // It can be called multiple times,
    // but intern code initializes v8 only once
    v8::V8::Initialize();

#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp_ini.remote_management_enable && oam != nullptr)
    {
        return SUCCESS;
    }
#endif

    load_plugins();

    if (!process_globals.snapshot_blob)
    {
        Platform::Initialize();
        Snapshot *snapshot = new Snapshot(process_globals.plugin_config, process_globals.plugin_src_list);
        Platform::Shutdown();
        if (!snapshot->IsOk())
        {
            delete snapshot;
        }
        else
        {
            process_globals.snapshot_blob = snapshot;
        }
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_v8)
{
<<<<<<< HEAD
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_v8, PHP_GSHUTDOWN(openrasp_v8));
=======
    if (process_globals.is_initialized && !OPENRASP_V8_G(is_isolate_initialized))
    {
        if (!process_globals.v8_platform)
        {
#ifdef ZTS
            process_globals.v8_platform = v8::platform::CreateDefaultPlatform();
#else
            process_globals.v8_platform = v8::platform::CreateDefaultPlatform(1);
#endif
            v8::V8::InitializePlatform(process_globals.v8_platform);
        }
        OPENRASP_V8_G(create_params).array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        OPENRASP_V8_G(create_params).snapshot_blob = &process_globals.snapshot_blob;
        OPENRASP_V8_G(create_params).external_references = external_references;

        v8::Isolate *isolate = v8::Isolate::New(OPENRASP_V8_G(create_params));
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        v8::Local<v8::String> key_action = V8STRING_I("action").ToLocalChecked();
        v8::Local<v8::String> key_message = V8STRING_I("message").ToLocalChecked();
        v8::Local<v8::String> key_name = V8STRING_I("name").ToLocalChecked();
        v8::Local<v8::String> key_confidence = V8STRING_I("confidence").ToLocalChecked();
        v8::Local<v8::String> key_algorithm = V8STRING_I("algorithm").ToLocalChecked();
        v8::Local<v8::Object> RASP = context->Global()
                                         ->Get(context, V8STRING_I("RASP").ToLocalChecked())
                                         .ToLocalChecked()
                                         .As<v8::Object>();
        v8::Local<v8::Function> check = RASP->Get(context, V8STRING_I("check").ToLocalChecked())
                                            .ToLocalChecked()
                                            .As<v8::Function>();

        OPENRASP_V8_G(isolate) = isolate;
        OPENRASP_V8_G(context).Reset(isolate, context);
        OPENRASP_V8_G(key_action).Reset(isolate, key_action);
        OPENRASP_V8_G(key_message).Reset(isolate, key_message);
        OPENRASP_V8_G(key_name).Reset(isolate, key_name);
        OPENRASP_V8_G(key_confidence).Reset(isolate, key_confidence);
        OPENRASP_V8_G(key_algorithm).Reset(isolate, key_algorithm);
        OPENRASP_V8_G(RASP).Reset(isolate, RASP);
        OPENRASP_V8_G(check).Reset(isolate, check);
        OPENRASP_V8_G(request_context).Reset(isolate, RequestContext::New(isolate));

        OPENRASP_V8_G(action_hash_ignore) = V8STRING_N("ignore").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_log) = V8STRING_N("log").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_block) = V8STRING_N("block").ToLocalChecked()->GetIdentityHash();
>>>>>>> master

    // Disposing v8 is permanent, it cannot be reinitialized,
    // it should generally not be necessary to dispose v8 before exiting a process,
    // so skip this step for module graceful reload
    // v8::V8::Dispose();
    Platform::Shutdown();
    delete process_globals.snapshot_blob;
    process_globals.snapshot_blob = nullptr;

    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_v8)
{
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp_ini.remote_management_enable && oam != nullptr)
    {
        uint64_t timestamp = oam->get_plugin_update_timestamp();
        if (!process_globals.snapshot_blob ||
            process_globals.snapshot_blob->IsExpired(timestamp))
        {
            std::unique_lock<std::mutex> lock(process_globals.mtx, std::try_to_lock);
            if (lock &&
                (!process_globals.snapshot_blob ||
                 process_globals.snapshot_blob->IsExpired(timestamp)))
            {
                std::string filename = std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("snapshot.dat");
                Snapshot *blob = new Snapshot(filename, timestamp);
                if (!blob->IsOk())
                {
                    delete blob;
                }
                else
                {
                    delete process_globals.snapshot_blob;
                    process_globals.snapshot_blob = blob;
                }
            }
        }
    }
#endif
    if (process_globals.snapshot_blob)
    {
        if (!OPENRASP_V8_G(isolate) || OPENRASP_V8_G(isolate)->IsExpired(process_globals.snapshot_blob->timestamp))
        {
            std::unique_lock<std::mutex> lock(process_globals.mtx, std::try_to_lock);
            if (lock)
            {
                if (OPENRASP_V8_G(isolate))
                {
                    OPENRASP_V8_G(isolate)->Dispose();
                }
                Platform::Initialize();
                OPENRASP_V8_G(isolate) = Isolate::New(process_globals.snapshot_blob);
            }
        }
    }
    if (OPENRASP_V8_G(isolate))
    {
        OPENRASP_V8_G(isolate)->GetData()->request_context.Reset();
    }
    return SUCCESS;
}
