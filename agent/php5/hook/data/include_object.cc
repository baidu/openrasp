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

#include "include_object.h"

namespace openrasp
{
namespace data
{

IncludeObject::IncludeObject(zval *filename, const std::string &document_root, const std::string &function, bool plugin_filter, bool without_protocol)
    : document_root(document_root), function(function)
{
    this->filename = filename;
    this->plugin_filter = plugin_filter;
    this->without_protocol = without_protocol;
    if (nullptr != filename && Z_TYPE_P(filename) == IS_STRING && Z_STRLEN_P(filename) > 0)
    {
        if (plugin_filter)
        {
            if (without_protocol)
            {
                param = std::string(Z_STRVAL_P(filename), Z_STRLEN_P(filename));
                if (openrasp::end_with(param, ".php") || openrasp::end_with(param, ".inc"))
                {
                    return;
                }
            }
        }
        realpath = openrasp_real_path(Z_STRVAL_P(filename), Z_STRLEN_P(filename), true, READING);
    }
}
std::string IncludeObject::build_lru_key() const
{
    return "";
}
OpenRASPCheckType IncludeObject::get_v8_check_type() const
{
    return INCLUDE;
}
bool IncludeObject::is_valid() const
{
    if (realpath.empty() || function.empty())
    {
        return false;
    }
    if (plugin_filter && 
        without_protocol && 
        param.find("../") == std::string::npos &&
        (!document_root.empty() && openrasp::start_with(realpath, document_root)))
    {
        return false;
    }
    return true;
}
void IncludeObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "path"), openrasp::NewV8String(isolate, Z_STRVAL_P(filename), Z_STRLEN_P(filename))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "url"), openrasp::NewV8String(isolate, Z_STRVAL_P(filename), Z_STRLEN_P(filename))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "realpath"), openrasp::NewV8String(isolate, realpath)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function)).IsJust();
}

} // namespace data

} // namespace openrasp