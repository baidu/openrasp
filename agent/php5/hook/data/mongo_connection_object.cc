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

#include "mongo_connection_object.h"
#include <sstream>

namespace openrasp
{
namespace data
{

bool MongoConnectionObject::is_valid() const
{
    if (hosts.empty() && sockets.empty())
    {
        return false;
    }
    return true;
}

void MongoConnectionObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, get_server())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "username"), openrasp::NewV8String(isolate, get_username())).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "connectionString"), openrasp::NewV8String(isolate, get_connection_string())).IsJust();

    size_t host_size = hosts.size();
    size_t port_size = ports.size();
    size_t socket_size = sockets.size();

    if (host_size > 1)
    {
        v8::Local<v8::Array> host_arr = v8::Array::New(isolate, host_size);
        for (int i = 0; i < host_size; i++)
        {
            host_arr->Set(context, i, NewV8String(isolate, hosts[i])).IsJust();
        }
        params->Set(context, openrasp::NewV8String(isolate, "hostnames"), host_arr).IsJust();

        v8::Local<v8::Array> port_arr = v8::Array::New(isolate, port_size);
        for (int i = 0; i < port_size; i++)
        {
            port_arr->Set(context, i, v8::Int32::New(isolate, ports[i])).IsJust();
        }
        params->Set(context, openrasp::NewV8String(isolate, "ports"), port_arr).IsJust();
    }

    if (socket_size > 1)
    {
        v8::Local<v8::Array> socket_arr = v8::Array::New(isolate, socket_size);
        for (int i = 0; i < socket_size; i++)
        {
            socket_arr->Set(context, i, NewV8String(isolate, sockets[i])).IsJust();
        }
        params->Set(context, openrasp::NewV8String(isolate, "sockets"), socket_arr).IsJust();
    }

    if (get_srv())
    {
        params->Set(context, openrasp::NewV8String(isolate, "dns"), openrasp::NewV8String(isolate, get_dns())).IsJust();
    }
}

//policy
void MongoConnectionObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"policy_params", "server"}, server);
    j.write_string({"policy_params", "username"}, username);
    j.write_string({"policy_params", "connectionString"}, connection_string);

    if (hosts.size() > 1)
    {
        j.write_vector({"policy_params", "hostnames"}, hosts);
        j.write_int64_vector({"policy_params", "ports"}, ports);
    }

    if (sockets.size() > 1)
    {
        j.write_vector({"policy_params", "sockets"}, sockets);
    }
    if (get_srv())
    {
        j.write_string({"policy_params", "dns"}, get_dns());
    }
}

void MongoConnectionObject::set_srv(bool srv)
{
    this->srv = srv;
}
bool MongoConnectionObject::get_srv() const
{
    return srv;
}

void MongoConnectionObject::set_dns(std::string dns)
{
    this->dns = dns;
}
std::string MongoConnectionObject::get_dns() const
{
    return dns;
}

bool MongoConnectionObject::parse(std::string uri)

{
    set_server("mongodb");
    static const std::string scheme = "mongodb://";
    std::size_t scheme_found = uri.find(scheme);
    if (scheme_found != std::string::npos)
    {
        uri.erase(0, scheme.length());
    }
    static const std::string scheme_srv = "mongodb+srv://";
    std::size_t scheme_srv_found = uri.find(scheme);
    if (scheme_srv_found != std::string::npos)
    {
        uri.erase(0, scheme_srv.length());
        set_srv(true);
    }
    std::string uph;
    std::size_t last_slash = uri.find_last_of("/");
    if (last_slash != std::string::npos)
    {
        uph = uri.substr(0, last_slash);
    }
    else
    {
        uph = uri;
    }
    std::string host_list;
    std::size_t at = uph.find_last_of("@");
    if (at != std::string::npos)
    {
        std::string up = uph.substr(0, at);
        parse_username_password(up);
        host_list = uph.substr(at + 1);
    }
    else
    {
        host_list = uph;
    }
    if (get_srv())
    {
        set_dns(host_list);
    }
    else
    {
        parse_host_list(host_list);
    }
    return true;
}

bool MongoConnectionObject::parse_username_password(std::string &usernamePassword)
{
    std::size_t colon = usernamePassword.find(":");
    if (colon != std::string::npos)
    {
        set_username(usernamePassword.substr(0, colon));
        set_password(usernamePassword.substr(colon));
    }
    return true;
}

bool MongoConnectionObject::parse_host_list(std::string &host_list)
{
    std::istringstream iss(host_list);
    std::string host_item;
    while (getline(iss, host_item, ','))
    {
        if (openrasp::end_with(host_item, ".sock"))
        {
            set_socket(host_item);
        }
        else
        {
            std::string host;
            int port = 27017;
            std::size_t open_bracket = host_item.find("[");
            std::size_t close_bracket = host_item.find("]");
            if (open_bracket != std::string::npos &&
                close_bracket != std::string::npos)
            {
                std::size_t colon = host_item.find(":", close_bracket);
                if (colon != std::string::npos)
                {
                    port = std::stoi(host_item.substr(colon + 1));
                }
                host = host_item.substr(1, close_bracket - open_bracket - 1);
            }
            else
            {
                std::size_t colon = host_item.find_last_of(":");
                if (colon != std::string::npos)
                {
                    if ((colon + 1) < host_item.size())
                    {
                        host = host_item.substr(0, colon);
                        port = std::stoi(host_item.substr(colon + 1));
                    }
                }
                else
                {
                    host = host_item;
                }
            }
            set_host(host);
            set_port(port);
        }
    }
    return true;
}

} // namespace data

} // namespace openrasp