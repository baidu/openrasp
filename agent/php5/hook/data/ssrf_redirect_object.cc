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

#include "ssrf_redirect_object.h"
#include "utils/net.h"
#include <curl/curl.h>

namespace openrasp
{
namespace data
{

SsrfRedirectObject::SsrfRedirectObject(zval *origin_url, zval *effective_url, const std::string &function, int curl_error, int http_status)
    : function(function)
{
    this->origin_url = origin_url;
    this->effective_url = effective_url;
    this->curl_error = curl_error;
    this->http_status = http_status;
    if (nullptr != origin_url && Z_TYPE_P(origin_url) == IS_STRING)
    {
        origin.parse(std::string(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url)));
    }
    if (nullptr != effective_url && Z_TYPE_P(effective_url) == IS_STRING)
    {
        effective.parse(std::string(Z_STRVAL_P(effective_url), Z_STRLEN_P(effective_url)));
    }
}
std::string SsrfRedirectObject::build_lru_key() const
{
    return "";
}
OpenRASPCheckType SsrfRedirectObject::get_v8_check_type() const
{
    return SSRF_REDIRECT;
}
bool SsrfRedirectObject::is_valid() const
{
    if (origin.has_error() || effective.has_error() || origin == effective)
    {
        return false;
    }
    return true;
}
void SsrfRedirectObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, function)).IsJust();

    params->Set(context, openrasp::NewV8String(isolate, "url"), openrasp::NewV8String(isolate, Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "hostname"), openrasp::NewV8String(isolate, origin.get_host())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "port"), openrasp::NewV8String(isolate, origin.get_port())).IsJust();
    std::vector<std::string> origin_ips = openrasp::lookup_host(origin.get_host());
    auto ip_arr = v8::Array::New(isolate);
    for (int i = 0; i < origin_ips.size(); ++i)
    {
        ip_arr->Set(context, i, openrasp::NewV8String(isolate, origin_ips[i])).IsJust();
    }
    params->Set(context, openrasp::NewV8String(isolate, "ip"), ip_arr).IsJust();

    params->Set(context, openrasp::NewV8String(isolate, "url2"), openrasp::NewV8String(isolate, Z_STRVAL_P(effective_url), Z_STRLEN_P(effective_url))).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "hostname2"), openrasp::NewV8String(isolate, effective.get_host())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "port2"), openrasp::NewV8String(isolate, effective.get_port())).IsJust();
    std::vector<std::string> effective_ips = openrasp::lookup_host(effective.get_host());
    auto ip2_arr = v8::Array::New(isolate);
    for (int i = 0; i < effective_ips.size(); ++i)
    {
        ip2_arr->Set(context, i, openrasp::NewV8String(isolate, effective_ips[i])).IsJust();
    }
    params->Set(context, openrasp::NewV8String(isolate, "ip2"), ip2_arr).IsJust();

    params->Set(context, openrasp::NewV8String(isolate, "http_status"), v8::Integer::New(isolate, curl_error == 0 ? http_status : 0)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "http_message"), openrasp::NewV8String(isolate, curl_error != 0 ? std::string(curl_easy_strerror((CURLcode)curl_error)) : "OK")).IsJust();
}

} // namespace data

} // namespace openrasp