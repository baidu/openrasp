#include "openrasp_v8.h"

namespace openrasp
{
using namespace std;
Exception::Exception(v8::Isolate *isolate, v8::TryCatch &try_catch) : string()
{
    string &ref = *this;
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(try_catch.Exception());
    v8::Local<v8::Message> message = try_catch.Message();
    if (message.IsEmpty())
    {
        ref.append(*exception).append("\n");
    }
    else
    {
        v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        int linenum = message->GetLineNumber(context).FromJust();
        ref.append(*filename).append(":").append(to_string(linenum)).append("\n");
        v8::String::Utf8Value sourceline(
            message->GetSourceLine(context).ToLocalChecked());
        ref.append(*sourceline).append("\n");
        int start = message->GetStartColumn(context).FromJust();
        int end = message->GetEndColumn(context).FromJust();
        ref.append(start, ' ').append(end - start, '^').append("\n");
        v8::Local<v8::Value> stack_trace_string;
        if (try_catch.StackTrace(context).ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString() &&
            v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0)
        {
            v8::String::Utf8Value stack_trace(stack_trace_string);
            ref.append(*stack_trace).append("\n");
        }
        else
        {
            ref.append(*exception).append("\n");
        }
    }
}
} // namespace openrasp_v8