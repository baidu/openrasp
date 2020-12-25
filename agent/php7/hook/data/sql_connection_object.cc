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

#include "sql_connection_object.h"

namespace openrasp
{
namespace data
{

SqlConnectionObject::SqlConnectionObject()
{
}

bool SqlConnectionObject::is_valid() const
{
    if (using_socket && sockets.empty())
    {
        return false;
    }
    if (!using_socket && hosts.empty())
    {
        return false;
    }
    return true;
}

std::string SqlConnectionObject::build_lru_key() const
{
    std::string result;
    result.append(server).append("-");
    if (!using_socket)
    {
        for (const std::string &host : hosts)
        {
            result.append(host).append("-");
        }
        for (const int port : ports)
        {
            result.append(std::to_string(port)).append("-");
        }
    }
    else
    {
        for (const std::string &socket : sockets)
        {
            result.append(socket).append("-");
        }
    }
    return result;
}

OpenRASPCheckType SqlConnectionObject::get_v8_check_type() const
{
    return DB_CONNECTION;
}

void SqlConnectionObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, get_server())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "username"), openrasp::NewV8String(isolate, get_username())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "connectionString"), openrasp::NewV8String(isolate, get_connection_string())).IsJust();

    size_t host_size = hosts.size();
    size_t port_size = ports.size();
    size_t socket_size = sockets.size();

    if (host_size == 1)
    {
        params->Set(context, openrasp::NewV8String(isolate, "hostname"), openrasp::NewV8String(isolate, hosts[0])).IsJust();
        params->Set(context, openrasp::NewV8String(isolate, "port"), v8::Integer::New(isolate, ports[0])).IsJust();
    }

    if (socket_size == 1)
    {
        params->Set(context, openrasp::NewV8String(isolate, "socket"), openrasp::NewV8String(isolate, sockets[0])).IsJust();
    }
}

void SqlConnectionObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"policy_params", "server"}, server);
    j.write_string({"policy_params", "username"}, username);
    j.write_string({"policy_params", "connectionString"}, connection_string);
    if (hosts.size() == 1)
    {
        j.write_string({"policy_params", "hostname"}, hosts[0]);
        j.write_int64({"policy_params", "port"}, ports[0]);
    }
    if (sockets.size() == 1)
    {
        j.write_string({"policy_params", "socket"}, sockets[0]);
    }
}

bool SqlConnectionObject::policy_check(JsonReader &j) const
{
    return false;
}

ulong SqlConnectionObject::hash() const
{
    return 0;
}

void SqlConnectionObject::set_host(const std::string &host)
{
    hosts.push_back(host);
}
std::vector<std::string> SqlConnectionObject::get_host() const
{
    return hosts;
}

void SqlConnectionObject::set_port(int port)
{
    ports.push_back(port);
}
std::vector<int> SqlConnectionObject::get_port() const
{
    return ports;
}

void SqlConnectionObject::set_socket(const std::string &socket)
{
    sockets.push_back(socket);
}
std::vector<std::string> SqlConnectionObject::get_socket() const
{
    return sockets;
}

void SqlConnectionObject::set_connection_string(const std::string &connection_string)
{
    this->connection_string = connection_string;
}
std::string SqlConnectionObject::get_connection_string() const
{
    return connection_string;
}

void SqlConnectionObject::set_server(const std::string &server)
{
    this->server = server;
}
std::string SqlConnectionObject::get_server() const
{
    return server;
}

void SqlConnectionObject::set_username(const std::string &username)
{
    this->username = username;
}
std::string SqlConnectionObject::get_username() const
{
    return username;
}

void SqlConnectionObject::set_password(const std::string &password)
{
    this->password = password;
}
std::string SqlConnectionObject::get_password() const
{
    return password;
}

void SqlConnectionObject::set_using_socket(bool using_socket)
{
    this->using_socket = using_socket;
}
bool SqlConnectionObject::get_using_socket() const
{
    return using_socket;
}

void SqlConnectionObject::set_name_value(const char *name, const char *val)
{
    if (strcmp(name, "user") == 0)
    {
        set_username(val);
    }
    else if (strcmp(name, "password") == 0)
    {
        set_password(val);
    }
    else if (strcmp(name, "host") == 0)
    {
        struct stat sb;
        if (stat(val, &sb) == 0)
        {
            set_socket(val);
            set_using_socket((sb.st_mode & S_IFDIR) != 0 || (sb.st_mode & S_IFSOCK) != 0);
        }
        else
        {
            set_host(val);
            set_using_socket(false);
        }
    }
    else if (strcmp(name, "port") == 0)
    {
        set_port(atoi(val));
    }
}

bool SqlConnectionObject::parse(std::string uri)
{
    return true;
}

} // namespace data

} // namespace openrasp