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

#include "fileupload_object.h"

namespace openrasp
{
namespace data
{

FileuploadObject::FileuploadObject(const openrasp::request::Parameter &parameter, zval *path, zval *dest, const std::string &content)
    : parameter(parameter), content(content)
{
    this->path = path;
    this->dest = dest;
    if (nullptr != dest && Z_TYPE_P(dest) == IS_STRING && Z_STRLEN_P(dest) > 0)
    {
        real_dest = openrasp_real_path(Z_STRVAL_P(dest), Z_STRLEN_P(dest), 0, WRITING);
    }
    if (nullptr != path && Z_TYPE_P(path) == IS_STRING && Z_STRLEN_P(path) > 0)
    {
        parameter.fetch_fileinfo_by_tmpname(std::string(Z_STRVAL_P(path), Z_STRLEN_P(path)), name, filename);
    }
}

std::string FileuploadObject::build_lru_key() const
{
    return "";
}

OpenRASPCheckType FileuploadObject::get_v8_check_type() const
{
    return FILE_UPLOAD;
}

bool FileuploadObject::is_valid() const
{
    if (real_dest.empty() ||
        name.empty() ||
        filename.empty())
    {
        return false;
    }
    return true;
}

void FileuploadObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "name"), openrasp::NewV8String(isolate, name)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "filename"), openrasp::NewV8String(isolate, filename)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "dest_path"), openrasp::NewV8String(isolate, Z_STRVAL_P(dest), Z_STRLEN_P(dest))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "dest_realpath"), openrasp::NewV8String(isolate, real_dest)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "content"), openrasp::NewV8String(isolate, content)).IsJust();
}

} // namespace data

} // namespace openrasp