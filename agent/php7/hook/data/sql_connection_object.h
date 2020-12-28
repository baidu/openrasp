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
#include "v8_material.h"

namespace openrasp
{
namespace data
{
class SqlConnectionObject : public PolicyMaterial, public V8Material
{
protected:
    std::vector<std::string> hosts;
    std::vector<int> ports;
    std::vector<std::string> sockets;
    std::string connection_string;
    std::string server;
    std::string username;
    std::string password;
    bool using_socket = false;

public:
    SqlConnectionObject();

    virtual bool is_valid() const;

    //v8
    virtual std::string build_lru_key() const;
    virtual OpenRASPCheckType get_v8_check_type() const;
    virtual void fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const;

    //policy
    virtual void fill_json_with_params(JsonReader &j) const;
    virtual bool policy_check(JsonReader &j) const;
    virtual ulong hash() const;

    //self
    virtual void set_host(const std::string &host);
    virtual std::vector<std::string> get_host() const;

    virtual void set_port(int port);
    virtual std::vector<int> get_port() const;

    virtual void set_socket(const std::string &socket);
    virtual std::vector<std::string> get_socket() const;

    virtual void set_connection_string(const std::string &connection_string);
    virtual std::string get_connection_string() const;

    virtual void set_server(const std::string &server);
    virtual std::string get_server() const;

    virtual void set_username(const std::string &username);
    virtual std::string get_username() const;

    virtual void set_password(const std::string &password);
    virtual std::string get_password() const;

    virtual void set_using_socket(bool using_socket);
    virtual bool get_using_socket() const;

    virtual void set_name_value(const char *name, const char *val);

    virtual bool parse(std::string uri);
};

} // namespace data

} // namespace openrasp