/*
 * Copyright 2017-2018 Baidu Inc.
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

#include "openrasp_mongo_entry.h"
#include "utils/string.h"
#include <sstream>

void MongoConnectionEntry::append_host_port(const std::string &host, int port)
{
  hosts.push_back(host);
  ports.push_back(port);
}

void MongoConnectionEntry::set_dns(std::string dns)
{
  this->dns = dns;
}

std::string MongoConnectionEntry::get_dns()
{
  return dns;
}

void MongoConnectionEntry::build_connection_params(zval *params, connection_policy_type type)
{
  if (params && Z_TYPE_P(params) == IS_ARRAY)
  {
    add_assoc_string(params, "server", (char *)get_server().c_str(), 1);
    {
      zval *host_arr = nullptr;
      MAKE_STD_ZVAL(host_arr);
      array_init(host_arr);
      for (auto host : hosts)
      {
        add_next_index_string(host_arr, (char *)host.c_str(), 1);
      }
      add_assoc_zval(params, "hostnames", host_arr);
    }
    add_assoc_string(params, "username", (char *)get_username().c_str(), 1);
    {
      zval *socket_arr = nullptr;
      MAKE_STD_ZVAL(socket_arr);
      array_init(socket_arr);
      for (auto socket : sockets)
      {
        add_next_index_string(socket_arr, (char *)socket.c_str(), 1);
      }
      add_assoc_zval(params, "sockets", socket_arr);
    }
    add_assoc_string(params, "connectionString", (char *)get_connection_string().c_str(), 1);
    {
      zval *port_arr = nullptr;
      MAKE_STD_ZVAL(port_arr);
      array_init(port_arr);
      for (int port : ports)
      {
        add_next_index_long(port_arr, port);
      }
      add_assoc_zval(params, "ports", port_arr);
    }
    if (connection_policy_type::PASSWORD == type)
    {
      add_assoc_string(params, "password", (char *)get_password().c_str(), 1);
    }
    if (get_srv())
    {
      add_assoc_string(params, "dns", (char *)get_dns().c_str(), 1);
    }
  }
}

void MongoConnectionEntry::append_socket(const std::string &socket)
{
  sockets.push_back(socket);
}

void MongoConnectionEntry::set_srv(bool srv)
{
  this->srv = srv;
}

bool MongoConnectionEntry::get_srv()
{
  return srv;
}

//the uri format must be correct cuz of successful connection
bool MongoConnectionEntry::parse(std::string uri)
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

bool MongoConnectionEntry::parse_username_password(std::string &usernamePassword)
{
  std::size_t colon = usernamePassword.find(":");
  if (colon != std::string::npos)
  {
    set_username(usernamePassword.substr(0, colon));
    set_password(usernamePassword.substr(colon));
  }
  return true;
}

bool MongoConnectionEntry::parse_host_list(std::string &host_list)
{
  std::istringstream iss(host_list);
  std::string host_item;
  while (getline(iss, host_item, ','))
  {
    if (openrasp::end_with(host_item, ".sock"))
    {
      append_socket(host_item);
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
      append_host_port(host, port);
    }
  }
  return true;
}
