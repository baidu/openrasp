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

#include "mongo_object.h"

namespace openrasp
{
namespace data
{

MongoObject::MongoObject(const std::string &server, const std::string &query, const std::string &classname, const std::string &method)
{
    this->server = server;
    this->query = query;
    this->classname = classname;
    this->method = method;
}
std::string MongoObject::build_lru_key() const
{
    return CheckTypeTransfer::instance().type_to_name(get_v8_check_type()) + classname + method + query;
}
OpenRASPCheckType MongoObject::get_v8_check_type() const
{
    return MONGO;
}
bool MongoObject::is_valid() const
{
    return !query.empty() && !classname.empty() && !method.empty();
}
void MongoObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "query"), openrasp::NewV8String(isolate, query)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "class"), openrasp::NewV8String(isolate, classname)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "method"), openrasp::NewV8String(isolate, method)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, server)).IsJust();
}

} // namespace data

} // namespace openrasp