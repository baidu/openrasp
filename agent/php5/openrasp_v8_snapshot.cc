#include "openrasp_v8.h"
#include "js/openrasp_v8_js.h"

namespace openrasp
{
Snapshot::Snapshot(const char *data, size_t raw_size, uint64_t timestamp)
{
    this->data = data;
    this->raw_size = raw_size;
    this->timestamp = timestamp;
}
Snapshot::Snapshot(const std::string &path, uint64_t timestamp) : Snapshot()
{
    char *buffer = nullptr;
    size_t size;
    std::ifstream file(path);
    if (file)
    {
        file.seekg(0, std::ios::end);
        size = file.tellg();
        file.seekg(0, std::ios::beg);
        if (size > 0)
        {
            buffer = new char[size];
            file.read(buffer, size);
        }
    }
    this->data = buffer;
    this->raw_size = size;
}
Snapshot::Snapshot(const std::string &config, const std::vector<openrasp_v8_js_src> &plugin_list) : Snapshot()
{
    TSRMLS_FETCH();
    v8::SnapshotCreator creator(external_references);
    Isolate *isolate = reinterpret_cast<Isolate *>(creator.GetIsolate());
#define DEFAULT_STACK_SIZE_IN_KB 1024
    uintptr_t current_stack = reinterpret_cast<uintptr_t>(&current_stack);
    uintptr_t stack_limit = current_stack - (DEFAULT_STACK_SIZE_IN_KB * 1024 / sizeof(uintptr_t));
    stack_limit = stack_limit < current_stack ? stack_limit : sizeof(stack_limit);
    isolate->SetStackLimit(stack_limit);
#undef DEFAULT_STACK_SIZE_IN_KB
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        v8::TryCatch try_catch;
        v8::Local<v8::Object> global = context->Global();
        global->Set(NewV8String(isolate, "global"), global);
        global->Set(NewV8String(isolate, "window"), global);
        v8::Local<v8::Function> log = v8::Function::New(isolate, v8native_log);
        v8::Local<v8::Object> v8_stdout = v8::Object::New(isolate);
        v8_stdout->Set(NewV8String(isolate, "write"), log);
        global->Set(NewV8String(isolate, "stdout"), v8_stdout);
        global->Set(NewV8String(isolate, "stderr"), v8_stdout);

        std::vector<openrasp_v8_js_src> internal_js_list = {
            openrasp_v8_js_src{"console.js", {reinterpret_cast<const char *>(console_js), console_js_len}},
            openrasp_v8_js_src{"checkpoint.js", {reinterpret_cast<const char *>(checkpoint_js), checkpoint_js_len}},
            openrasp_v8_js_src{"error.js", {reinterpret_cast<const char *>(error_js), error_js_len}},
            openrasp_v8_js_src{"context.js", {reinterpret_cast<const char *>(context_js), context_js_len}},
            openrasp_v8_js_src{"sql_tokenize.js", {reinterpret_cast<const char *>(sql_tokenize_js), sql_tokenize_js_len}},
            openrasp_v8_js_src{"rasp.js", {reinterpret_cast<const char *>(rasp_js), rasp_js_len}},
        };
        for (auto &js_src : internal_js_list)
        {
            if (isolate->ExecScript(js_src.source, js_src.filename).IsEmpty())
            {
                Exception e(isolate, try_catch);
                LOG_G(plugin_logger).log(LEVEL_INFO, e.c_str(), e.length() TSRMLS_CC);
                openrasp_error(E_WARNING, PLUGIN_ERROR, _("Fail to initialize js plugin - %s"), e.c_str());
                // no need to continue
                return;
            }
        }
        if (isolate->ExecScript(config, "config.js").IsEmpty())
        {
            Exception e(isolate, try_catch);
            LOG_G(plugin_logger).log(LEVEL_INFO, e.c_str(), e.length() TSRMLS_CC);
        }
        for (auto &plugin_src : plugin_list)
        {
            if (isolate->ExecScript("(function(){\n" + plugin_src.source + "\n})()", plugin_src.filename, -1).IsEmpty())
            {
                Exception e(isolate, try_catch);
                LOG_G(plugin_logger).log(LEVEL_INFO, e.c_str(), e.length() TSRMLS_CC);
            }
        }
        creator.SetDefaultContext(context);
    }
    v8::StartupData snapshot = creator.CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
    this->data = snapshot.data;
    this->raw_size = snapshot.raw_size;
}
bool Snapshot::Save(const std::string &path) const
{
    std::ofstream file(path);
    if (file)
    {
        file.write(data, raw_size);
        return true;
    }
    // check errno when return value is false
    return false;
}
void Snapshot::v8native_log(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    for (int i = 0; i < info.Length(); i++)
    {
        v8::String::Utf8Value message(info[i]);
        LOG_G(plugin_logger).log(LEVEL_INFO, *message, message.length() TSRMLS_CC);
    }
}
} // namespace openrasp