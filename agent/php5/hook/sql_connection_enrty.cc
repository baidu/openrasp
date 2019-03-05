/*
 * Copyright 2017-2019 Baidu Inc.
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

#include "openrasp_sql.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void SqlConnectionEntry::set_connection_string(std::string connection_string)
{
  this->connection_string = connection_string;
}
std::string SqlConnectionEntry::get_connection_string() const
{
  return connection_string;
}

void SqlConnectionEntry::set_server(std::string server)
{
  this->server = server;
}
std::string SqlConnectionEntry::get_server() const
{
  return server;
}

void SqlConnectionEntry::set_host(std::string host)
{
  this->host = host;
}
std::string SqlConnectionEntry::get_host() const
{
  return host;
}

void SqlConnectionEntry::set_socket(std::string socket)
{
  this->socket = socket;
}
std::string SqlConnectionEntry::get_socket() const
{
  return socket;
}

void SqlConnectionEntry::set_username(std::string username)
{
  this->username = username;
}
std::string SqlConnectionEntry::get_username() const
{
  return username;
}

void SqlConnectionEntry::set_port(int port)
{
  this->port = port;
}
int SqlConnectionEntry::get_port() const
{
  return port;
}

std::string SqlConnectionEntry::build_policy_msg()
{
  std::ostringstream oss;
  oss << "Database security - Connecting to a "
      << server
      << " instance using the high privileged account: "
      << username;
  if (get_using_socket())
  {
    oss << " (via unix domain socket)";
  }
  return oss.str();
}

ulong SqlConnectionEntry::build_hash_code()
{
  std::string server_host_port = server + "-" + host + ":" +
                                 (get_using_socket() ? socket : std::to_string(port));
  return zend_inline_hash_func(server_host_port.c_str(), server_host_port.length());
}

void SqlConnectionEntry::set_using_socket(bool using_socket)
{
  this->using_socket = using_socket;
}

bool SqlConnectionEntry::get_using_socket() const
{
  return using_socket;
}

void SqlConnectionEntry::set_name_value(const char *name, const char *val)
{
  if (strcmp(name, "user") == 0)
  {
    set_username(val);
  }
  else if (strcmp(name, "host") == 0)
  {
    set_host(val);
    struct stat sb;
    if (stat(val, &sb) == 0)
    {
      set_using_socket((sb.st_mode & S_IFDIR) != 0 || (sb.st_mode & S_IFSOCK) != 0);
    }
    else
    {
      set_using_socket(false);
    }
  }
  else if (strcmp(name, "port") == 0)
  {
    set_port(atoi(val));
  }
}