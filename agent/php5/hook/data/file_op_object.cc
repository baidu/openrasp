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

#include "file_op_object.h"

namespace openrasp
{
namespace data
{

FileOpObject::FileOpObject(zval *file, PathOperation w_op, bool use_include_path)
{
    this->file = file;
    this->w_op = w_op;
    this->use_include_path = use_include_path;
    if (nullptr != file && Z_TYPE_P(file) == IS_STRING && Z_STRLEN_P(file) > 0)
    {
        realpath = openrasp_real_path(Z_STRVAL_P(file), Z_STRLEN_P(file), use_include_path, w_op);
    }
}

std::string FileOpObject::build_lru_key() const
{
    return CheckTypeTransfer::instance().type_to_name(get_v8_check_type()) + realpath;
}

OpenRASPCheckType FileOpObject::get_v8_check_type() const
{
    switch (w_op)
    {
    case PathOperation::OPENDIR:
        return OpenRASPCheckType::DIRECTORY;
        break;
    case PathOperation::WRITING:
        return OpenRASPCheckType::WRITE_FILE;
        break;
    case PathOperation::UNLINK:
        return OpenRASPCheckType::DELETE_FILE;
        break;
    case PathOperation::READING:
    default:
        return OpenRASPCheckType::READ_FILE;
        break;
    }
}
bool FileOpObject::is_valid() const
{
    return !realpath.empty();
}
void FileOpObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, Z_STRVAL_P(file), Z_STRLEN_P(file))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, realpath)).IsJust();
}

} // namespace data

} // namespace openrasp