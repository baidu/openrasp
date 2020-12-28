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

#include "policy_material.h"
#include "sql_connection_object.h"

namespace openrasp
{
namespace data
{
class MongoConnectionObject : public SqlConnectionObject
{
protected:
    std::string dns;
    bool srv = false;

public:
    MongoConnectionObject() = default;

    virtual bool is_valid() const;

    //v8
    virtual void fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const;

    //policy
    virtual void fill_json_with_params(JsonReader &j) const;

    virtual void set_srv(bool srv);
    virtual bool get_srv() const;

    virtual void set_dns(std::string dns);
    virtual std::string get_dns() const;

    virtual bool parse(std::string uri);
    virtual bool parse_username_password(std::string &usernamePassword);
    virtual bool parse_host_list(std::string &host_list);
};

} // namespace data

} // namespace openrasp