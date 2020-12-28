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

#pragma once

#include "php_openrasp.h"
#include "openrasp_v8.h"
#include "v8_material.h"

namespace openrasp
{
namespace data
{

class ResponseObject : public V8Material
{
private:
    const char *content;
    size_t content_length;
    const char *content_type;

public:
    ResponseObject(const char *content, size_t content_length, const char *content_type) : content(content), content_length(content_length), content_type(content_type) {}
    virtual bool is_valid() const
    {
        if (strlen(content_type) > 0 &&
            (strstr(content_type, "video") != nullptr || strstr(content_type, "audio") != nullptr || strstr(content_type, "image") != nullptr))
        {
            return false;
        }
        return true;
    };
    virtual std::string build_lru_key() const { return ""; };
    virtual OpenRASPCheckType get_v8_check_type() const { return OpenRASPCheckType::RESPONSE; };
    virtual void fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
    {
        v8::HandleScope handle_scope(isolate);
        auto context = isolate->GetCurrentContext();
        params->Set(context, openrasp_v8::NewV8String(isolate, "content"), openrasp_v8::NewV8String(isolate, content, content_length)).IsJust();
        params->Set(context, openrasp_v8::NewV8String(isolate, "content_type"), openrasp_v8::NewV8String(isolate, content_type)).IsJust();
        params->Set(context, openrasp_v8::NewV8String(isolate, "stack"), v8::Array::New(isolate)).IsJust();
    };
};

} // namespace data

} // namespace openrasp