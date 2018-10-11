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
#include "php_scandir.h"
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

static inline void load_plugins(TSRMLS_D);
static inline bool init_isolate(TSRMLS_D);
static inline bool shutdown_isolate(TSRMLS_D);

bool openrasp_check(v8::Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params TSRMLS_DC)
{
    auto context = isolate->GetCurrentContext();
    v8::TryCatch try_catch;
    auto check = OPENRASP_V8_G(check).Get(isolate);
    auto request_context = OPENRASP_V8_G(request_context).Get(isolate);
    v8::Local<v8::Value> argv[]{type, params, request_context};

    v8::Local<v8::Value> rst;
    auto task = new TimeoutTask(isolate, openrasp_ini.timeout_ms);
    task->GetMtx().lock();
    process_globals.v8_platform->CallOnBackgroundThread(task, v8::Platform::kShortRunningTask);
    (void)check->Call(context, check, 3, argv).ToLocal(&rst);
    task->GetMtx().unlock();
    if (UNLIKELY(rst.IsEmpty()))
    {
        if (try_catch.Message().IsEmpty())
        {
            auto console_log = OPENRASP_V8_G(console_log).Get(isolate);
            auto message = v8::Object::New(isolate);
            message->Set(V8STRING_N("message").ToLocalChecked(), V8STRING_N("Javascript plugin execution timeout.").ToLocalChecked());
            message->Set(V8STRING_N("type").ToLocalChecked(), type);
            message->Set(V8STRING_N("params").ToLocalChecked(), params);
            message->Set(V8STRING_N("context").ToLocalChecked(), request_context);
            (void)console_log->Call(context, console_log, 1, reinterpret_cast<v8::Local<v8::Value> *>(&message)).IsEmpty();
        }
        else
        {
            std::stringstream stream;
            v8error_to_stream(isolate, try_catch, stream);
            auto error = stream.str();
            plugin_info(error.c_str(), error.length() TSRMLS_CC);
        }
        return 0;
    }
    if (UNLIKELY(!rst->IsArray()))
    {
        return 0;
    }
    auto key_action = OPENRASP_V8_G(key_action).Get(isolate);
    auto key_message = OPENRASP_V8_G(key_message).Get(isolate);
    auto key_name = OPENRASP_V8_G(key_name).Get(isolate);
    auto key_confidence = OPENRASP_V8_G(key_confidence).Get(isolate);
    auto JSON_stringify = OPENRASP_V8_G(JSON_stringify).Get(isolate);

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
        if (LIKELY(OPENRASP_V8_G(action_hash_ignore) == action_hash))
        {
            continue;
        }
        is_block = is_block || OPENRASP_V8_G(action_hash_block) == action_hash;

        alarm_info(isolate, type, params, item TSRMLS_CC);
    }
    return is_block;
}

unsigned char openrasp_check(const char *c_type, zval *z_params TSRMLS_DC)
{
    v8::Isolate *isolate = get_isolate(TSRMLS_C);
    if (UNLIKELY(!isolate))
    {
        return 0;
    }
    v8::HandleScope handlescope(isolate);

    auto type = V8STRING_N(c_type).ToLocalChecked();
    auto params = v8::Local<v8::Object>::Cast(zval_to_v8val(z_params, isolate TSRMLS_CC));

    return openrasp_check(isolate, type, params TSRMLS_CC);
}

// static inline bool init_platform(TSRMLS_D)
bool init_platform(TSRMLS_D)
{
    if (!process_globals.v8_platform)
    {
        process_globals.v8_platform = v8::platform::CreateDefaultPlatform(1);
        v8::V8::InitializePlatform(process_globals.v8_platform);
    }
    return true;
}

// static inline bool shutdown_platform(TSRMLS_D)
bool shutdown_platform(TSRMLS_D)
{
    if (process_globals.v8_platform)
    {
        v8::V8::ShutdownPlatform();
        delete process_globals.v8_platform;
        process_globals.v8_platform = nullptr;
    }
    return true;
}

static inline bool init_isolate(TSRMLS_D)
{
    if (!OPENRASP_V8_G(is_isolate_initialized))
    {
        OPENRASP_V8_G(create_params).array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        OPENRASP_V8_G(create_params).snapshot_blob = process_globals.snapshot_blob;
        OPENRASP_V8_G(create_params).external_references = external_references;

        v8::Isolate *isolate = v8::Isolate::New(OPENRASP_V8_G(create_params));
        isolate->Enter();
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        context->Enter();
        v8::Local<v8::String> key_action = V8STRING_I("action").ToLocalChecked();
        v8::Local<v8::String> key_message = V8STRING_I("message").ToLocalChecked();
        v8::Local<v8::String> key_name = V8STRING_I("name").ToLocalChecked();
        v8::Local<v8::String> key_confidence = V8STRING_I("confidence").ToLocalChecked();
        v8::Local<v8::Object> RASP = context->Global()
                                         ->Get(context, V8STRING_I("RASP").ToLocalChecked())
                                         .ToLocalChecked()
                                         .As<v8::Object>();
        v8::Local<v8::Function> check = RASP->Get(context, V8STRING_I("check").ToLocalChecked())
                                            .ToLocalChecked()
                                            .As<v8::Function>();
        auto console_log = context->Global()
                               ->Get(v8::String::NewFromUtf8(isolate, "console"))
                               .As<v8::Object>()
                               ->Get(v8::String::NewFromUtf8(isolate, "log"))
                               .As<v8::Function>();
        auto JSON_stringify = context->Global()
                                  ->Get(v8::String::NewFromUtf8(isolate, "JSON"))
                                  .As<v8::Object>()
                                  ->Get(v8::String::NewFromUtf8(isolate, "stringify"))
                                  .As<v8::Function>();

        OPENRASP_V8_G(isolate) = isolate;
        OPENRASP_V8_G(context).Reset(isolate, context);
        OPENRASP_V8_G(key_action).Reset(isolate, key_action);
        OPENRASP_V8_G(key_message).Reset(isolate, key_message);
        OPENRASP_V8_G(key_name).Reset(isolate, key_name);
        OPENRASP_V8_G(key_confidence).Reset(isolate, key_confidence);
        OPENRASP_V8_G(RASP).Reset(isolate, RASP);
        OPENRASP_V8_G(check).Reset(isolate, check);
        OPENRASP_V8_G(request_context).Reset(isolate, RequestContext::New(isolate));
        OPENRASP_V8_G(console_log).Reset(isolate, console_log);
        OPENRASP_V8_G(JSON_stringify).Reset(isolate, JSON_stringify);

        OPENRASP_V8_G(action_hash_ignore) = V8STRING_N("ignore").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_log) = V8STRING_N("log").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_block) = V8STRING_N("block").ToLocalChecked()->GetIdentityHash();

        OPENRASP_V8_G(is_isolate_initialized) = true;
    }
    return true;
}

static inline bool shutdown_isolate(TSRMLS_D)
{
    if (OPENRASP_V8_G(is_isolate_initialized))
    {
        auto isolate = OPENRASP_V8_G(isolate);
        {
            v8::HandleScope handle_scope(isolate);
            auto context = OPENRASP_V8_G(context).Get(isolate);
            context->Exit();
        }
        isolate->Exit();

        OPENRASP_V8_G(context).Reset();
        OPENRASP_V8_G(key_action).Reset();
        OPENRASP_V8_G(key_message).Reset();
        OPENRASP_V8_G(key_name).Reset();
        OPENRASP_V8_G(key_confidence).Reset();
        OPENRASP_V8_G(RASP).Reset();
        OPENRASP_V8_G(check).Reset();
        OPENRASP_V8_G(request_context).Reset();
        OPENRASP_V8_G(console_log).Reset();
        OPENRASP_V8_G(JSON_stringify).Reset();

        OPENRASP_V8_G(isolate)->Dispose();
        OPENRASP_V8_G(isolate) = nullptr;
        delete OPENRASP_V8_G(create_params).array_buffer_allocator;
        OPENRASP_V8_G(is_isolate_initialized) = false;
    }
    return true;
}

v8::Isolate *get_isolate(TSRMLS_D)
{
    if (UNLIKELY(!process_globals.v8_platform))
    {
        init_platform(TSRMLS_C);
    }

#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp_ini.remote_management_enable && oam != nullptr)
    {
        uint64_t timestamp = oam->get_plugin_update_timestamp();
        if (process_globals.snapshot_blob->IsExpired(timestamp))
        {
            if (process_globals.mtx.try_lock() &&
                process_globals.snapshot_blob->IsExpired(timestamp))
            {
                std::string filename = std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("snapshot.dat");
                StartupData *blob = new StartupData(filename, timestamp);
                if (blob->IsOk())
                {
                    delete process_globals.snapshot_blob;
                    process_globals.snapshot_blob = blob;
                }
                process_globals.mtx.unlock();
            }
        }
    }
#endif

    if (UNLIKELY(process_globals.snapshot_blob &&
                 (!OPENRASP_V8_G(is_isolate_initialized) || process_globals.snapshot_blob->timestamp > OPENRASP_V8_G(plugin_update_timestamp))))
    {
        if (process_globals.mtx.try_lock() &&
            process_globals.snapshot_blob &&
            (!OPENRASP_V8_G(is_isolate_initialized) || process_globals.snapshot_blob->timestamp > OPENRASP_V8_G(plugin_update_timestamp)))
        {
            shutdown_isolate(TSRMLS_C);
            init_isolate(TSRMLS_C);
            OPENRASP_V8_G(plugin_update_timestamp) = process_globals.snapshot_blob->timestamp;
            process_globals.mtx.unlock();
        }
    }

    return OPENRASP_V8_G(isolate);
}

static inline void load_plugins(TSRMLS_D)
{
    std::vector<openrasp_v8_js_src> plugin_src_list;
    std::string plugin_path(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("plugins"));
    dirent **ent = nullptr;
    int n_plugin = php_scandir(plugin_path.c_str(), &ent, nullptr, nullptr);
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
                    plugin_src_list.emplace_back(openrasp_v8_js_src{filename, source});
                }
                else
                {
                    openrasp_error(E_WARNING, CONFIG_ERROR, _("Ignored Javascript plugin file '%s', as it exceeds 10 MB in file size."), filename.c_str());
                }
            }
        }
        free(ent[i]);
    }
    free(ent);
    process_globals.plugin_src_list = plugin_src_list;
}

} // namespace openrasp

using namespace openrasp;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8)
{
#ifdef ZTS
    new (openrasp_v8_globals) _zend_openrasp_v8_globals;
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp_v8)
{
    shutdown_isolate(TSRMLS_C);
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

    load_plugins(TSRMLS_C);

    if (!process_globals.snapshot_blob)
    {
        init_platform(TSRMLS_C);
        StartupData *snapshot = get_snapshot(TSRMLS_C);
        shutdown_platform(TSRMLS_C);
        if (snapshot->IsOk())
        {
            process_globals.snapshot_blob = snapshot;
        }
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_v8)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_v8, PHP_GSHUTDOWN(openrasp_v8));

    // Disposing v8 is permanent, it cannot be reinitialized,
    // it should generally not be necessary to dispose v8 before exiting a process,
    // so skip this step for module graceful reload
    // v8::V8::Dispose();
    shutdown_platform(TSRMLS_C);
    delete process_globals.snapshot_blob;

    return SUCCESS;
}
