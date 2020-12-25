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

#include "copy_object.h"

namespace openrasp
{
namespace data
{

CopyObject::CopyObject(zval *source, zval *target)
{
    this->source = source;
    this->target = target;
    if (nullptr != source && Z_TYPE_P(source) == IS_STRING && Z_STRLEN_P(source) > 0)
    {
        source_realpath = openrasp_real_path(Z_STRVAL_P(source), Z_STRLEN_P(source), false, READING);
    }
    if (nullptr != target && Z_TYPE_P(target) == IS_STRING && Z_STRLEN_P(target) > 0)
    {
        target_realpath = openrasp_real_path(Z_STRVAL_P(target), Z_STRLEN_P(target), false, WRITING);
    }
}
bool CopyObject::is_valid() const
{
    return !source_realpath.empty() && !target_realpath.empty();
}

//v8
std::string CopyObject::build_lru_key() const
{
    return CheckTypeTransfer::instance().type_to_name(get_v8_check_type()) + source_realpath + target_realpath;
}
OpenRASPCheckType CopyObject::get_v8_check_type() const
{
    return COPY;
}
void CopyObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_realpath)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, target_realpath)).IsJust();
}

} // namespace data

} // namespace openrasp