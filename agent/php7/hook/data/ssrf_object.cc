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

#include "ssrf_object.h"
#include "utils/net.h"

namespace openrasp
{
namespace data
{

SsrfObject::SsrfObject(const std::string &function_name, zval *origin_url)
{
    this->function_name = function_name;
    this->origin_url = origin_url;
    if (nullptr != origin_url && Z_TYPE_P(origin_url) == IS_STRING)
    {
        url.parse(std::string(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url)));
    }
}
std::string SsrfObject::build_lru_key() const
{
    return CheckTypeTransfer::instance().type_to_name(get_v8_check_type()) + function_name + std::string(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url));
}

OpenRASPCheckType SsrfObject::get_v8_check_type() const
{
    return SSRF;
}

bool SsrfObject::is_valid() const
{
    if (url.has_error())
    {
        return false;
    }
    return true;
}

void SsrfObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "url"), openrasp::NewV8String(isolate, Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function_name)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "hostname"), openrasp::NewV8String(isolate, url.get_host())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "port"), openrasp::NewV8String(isolate, url.get_port())).IsJust();
    std::vector<std::string> ips = openrasp::lookup_host(url.get_host());
    auto ip_arr = v8::Array::New(isolate);
    for (int i = 0; i < ips.size(); ++i)
    {
        ip_arr->Set(context, i, openrasp::NewV8String(isolate, ips[i])).IsJust();
    }
    params->Set(context, openrasp::NewV8String(isolate, "ip"), ip_arr).IsJust();
}

} // namespace data

} // namespace openrasp