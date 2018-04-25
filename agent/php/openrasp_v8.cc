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

extern "C" {
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_openrasp.h"
#include "php_scandir.h"
}
#include <sstream>
#include <fstream>
#include "openrasp_v8.h"
#include "js/openrasp_v8_js.h"
#include "openrasp_ini.h"

using namespace openrasp;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_v8)

openrasp_v8_process_globals openrasp::process_globals;

void openrasp_load_plugins(TSRMLS_D);
static bool init_isolate(TSRMLS_D);
static bool shutdown_isolate(TSRMLS_D);

unsigned char openrasp_check(const char *c_type, zval *z_params TSRMLS_DC)
{
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
        process_globals.v8_platform->CallOnBackgroundThread(task);
        bool avoidwarning = check->Call(context, check, 3, argv).ToLocal(&rst);
    }
    if (rst.IsEmpty())
    {
        if (try_catch.Message().IsEmpty())
        {
            plugin_info(ZEND_STRL("Check Timeout") TSRMLS_CC);
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
        v8::String::Utf8Value utf_action(v8_action);
        v8::String::Utf8Value utf_message(v8_message);
        v8::String::Utf8Value utf_name(v8_name);

        zval z_type, z_action, z_message, z_name, z_confidence;
        INIT_ZVAL(z_type);
        INIT_ZVAL(z_action);
        INIT_ZVAL(z_message);
        INIT_ZVAL(z_name);
        INIT_ZVAL(z_confidence);
        ZVAL_STRING(&z_type, c_type, 0);
        ZVAL_STRINGL(&z_action, *utf_action, utf_action.length(), 0);
        ZVAL_STRINGL(&z_message, *utf_message, utf_message.length(), 0);
        ZVAL_STRINGL(&z_name, *utf_name, utf_name.length(), 0);
        ZVAL_LONG(&z_confidence, v8_confidence->Int32Value());

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
        alarm_info(&result TSRMLS_CC);
        zval_dtor(&result);
    }
    return is_block ? 1 : 0;
}

static void v8native_log(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    for (int i = 0; i < info.Length(); i++)
    {
        v8::String::Utf8Value message(info[i]);
        plugin_info(*message, message.length() TSRMLS_CC);
    }
}

static void v8native_antlr4(const v8::FunctionCallbackInfo<v8::Value> &info)
{
#ifdef HAVE_NATIVE_ANTLR4
    static TokenizeErrorListener tokenizeErrorListener;
    if (info.Length() >= 1 && info[0]->IsString())
    {
        antlr4::ANTLRInputStream input(*v8::String::Utf8Value(info[0]));
        SQLLexer lexer(&input);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&tokenizeErrorListener);
        antlr4::CommonTokenStream output(&lexer);
        output.fill();
        auto tokens = output.getTokens();
        int length = tokens.size();
        v8::Isolate *isolate = info.GetIsolate();
        v8::Local<v8::Array> arr = v8::Array::New(isolate, length - 1);
        for (int i = 0; i < length - 1; i++)
        {
            v8::Local<v8::String> token;
            if (V8STRING_N(tokens[i]->getText().c_str()).ToLocal(&token))
            {
                arr->Set(i, token);
            }
        }
        info.GetReturnValue().Set(arr);
    }
#endif
}

intptr_t external_references[] = {
    reinterpret_cast<intptr_t>(v8native_log),
    reinterpret_cast<intptr_t>(v8native_antlr4),
    0,
};

static v8::StartupData init_js_snapshot(TSRMLS_D)
{
    v8::SnapshotCreator creator(external_references);
    v8::Isolate *isolate = creator.GetIsolate();
#ifdef PHP_WIN32
    uintptr_t current_stack = reinterpret_cast<uintptr_t>(&current_stack);
    uintptr_t stack_limit = current_stack - 512 * 1024;
    stack_limit = stack_limit < current_stack ? stack_limit : sizeof(stack_limit);
    isolate->SetStackLimit(stack_limit);
#endif
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        v8::TryCatch try_catch;
        v8::Local<v8::Object> global = context->Global();
        global->Set(V8STRING_I("global").ToLocalChecked(), global);
        v8::Local<v8::Function> log = v8::Function::New(isolate, v8native_log);
        v8::Local<v8::Object> v8_stdout = v8::Object::New(isolate);
        v8_stdout->Set(V8STRING_I("write").ToLocalChecked(), log);
        global->Set(V8STRING_I("stdout").ToLocalChecked(), v8_stdout);
        global->Set(V8STRING_I("stderr").ToLocalChecked(), v8_stdout);

#define MAKE_JS_SRC_PAIR(name) {{(const char *)name##_js, name##_js_len}, ZEND_TOSTR(name) ".js"}
        std::vector<std::pair<std::string, std::string>> js_src_list = {
            MAKE_JS_SRC_PAIR(console),
            MAKE_JS_SRC_PAIR(checkpoint),
            MAKE_JS_SRC_PAIR(error),
            MAKE_JS_SRC_PAIR(context),
            MAKE_JS_SRC_PAIR(sql_tokenize),
            MAKE_JS_SRC_PAIR(rasp),
        };
        for (auto js_src : js_src_list)
        {
            if (exec_script(isolate, context, std::move(js_src.first), std::move(js_src.second)).IsEmpty())
            {
                std::stringstream stream;
                v8error_to_stream(isolate, try_catch, stream);
                std::string error = stream.str();
                plugin_info(error.c_str(), error.length() TSRMLS_CC);
                openrasp_error(E_WARNING, PLUGIN_ERROR, _("Fail to initialize js plugin - %s"), error.c_str());
                return v8::StartupData{nullptr, 0};
            }
        }
#ifdef HAVE_NATIVE_ANTLR4
        v8::Local<v8::Function> sql_tokenize = v8::Function::New(isolate, v8native_antlr4);
        context->Global()
            ->Get(context, V8STRING_I("RASP").ToLocalChecked())
            .ToLocalChecked()
            .As<v8::Object>()
            ->Set(V8STRING_I("sql_tokenize").ToLocalChecked(), sql_tokenize);
#endif
        openrasp_load_plugins(TSRMLS_C);
        for (auto plugin_src : process_globals.plugin_src_list)
        {
            if (exec_script(isolate, context, "(function(){\n" + plugin_src.source + "\n})()", plugin_src.filename, -1).IsEmpty())
            {
                std::stringstream stream;
                v8error_to_stream(isolate, try_catch, stream);
                std::string error = stream.str();
                plugin_info(error.c_str(), error.length() TSRMLS_CC);
            }
        }
        creator.SetDefaultContext(context);
    }
    return creator.CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
}

static bool init_isolate(TSRMLS_D)
{
    if (process_globals.is_initialized && !OPENRASP_V8_G(is_isolate_initialized))
    {
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
        OPENRASP_V8_G(RASP).Reset(isolate, RASP);
        OPENRASP_V8_G(check).Reset(isolate, check);
        OPENRASP_V8_G(request_context).Reset(isolate, RequestContext::New(isolate));

        OPENRASP_V8_G(action_hash_ignore) = V8STRING_N("ignore").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_log) = V8STRING_N("log").ToLocalChecked()->GetIdentityHash();
        OPENRASP_V8_G(action_hash_block) = V8STRING_N("block").ToLocalChecked()->GetIdentityHash();

        OPENRASP_V8_G(is_isolate_initialized) = true;
    }
    return OPENRASP_V8_G(is_isolate_initialized);
}

static bool shutdown_isolate(TSRMLS_D)
{
    if (OPENRASP_V8_G(is_isolate_initialized))
    {
        OPENRASP_V8_G(isolate)->Dispose();
        delete OPENRASP_V8_G(create_params).array_buffer_allocator;
        OPENRASP_V8_G(is_isolate_initialized) = false;
    }
    return true;
}

void openrasp_load_plugins(TSRMLS_D)
{
    std::vector<openrasp_v8_plugin_src> plugin_src_list;
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
                    plugin_src_list.push_back(openrasp_v8_plugin_src{filename, source});
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
    process_globals.plugin_src_list = std::move(plugin_src_list);
}

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
    // It can be called multiple times,
    // but intern code initializes v8 only once
    v8::V8::Initialize();

    V8Platform platform;
    v8::V8::InitializePlatform(&platform);
    process_globals.snapshot_blob = init_js_snapshot(TSRMLS_C);
    v8::V8::ShutdownPlatform();
    if (process_globals.snapshot_blob.data == nullptr ||
        process_globals.snapshot_blob.raw_size <= 0)
    {
        return FAILURE;
    }

    process_globals.v8_platform = new V8Platform();
    v8::V8::InitializePlatform(process_globals.v8_platform);
    process_globals.is_initialized = true;

    ZEND_INIT_MODULE_GLOBALS(openrasp_v8, PHP_GINIT(openrasp_v8), PHP_GSHUTDOWN(openrasp_v8));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_v8)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_v8, PHP_GSHUTDOWN(openrasp_v8));
    if (process_globals.is_initialized)
    {
        // Disposing v8 is permanent, it cannot be reinitialized,
        // it should generally not be necessary to dispose v8 before exiting a process,
        // so skip this step for module graceful reload
        // v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        delete[] process_globals.snapshot_blob.data;
        process_globals.snapshot_blob.data = nullptr;
        delete process_globals.v8_platform;
        process_globals.v8_platform = nullptr;
        process_globals.is_initialized = false;
    }
    return SUCCESS;
}
