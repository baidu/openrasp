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

#ifndef OPENRASP_MONGO_ENTRY_H
#define OPENRASP_MONGO_ENTRY_H

#include "openrasp_sql.h"

class MongoConnectionEntry : public SqlConnectionEntry
{
protected:
  std::vector<std::string> hosts;
  std::vector<int> ports;
  std::vector<std::string> sockets;
  std::string dns;
  bool srv = false;

public:
  virtual void append_host_port(const std::string &host, int port);
  virtual void append_socket(const std::string &socket);
  virtual void build_connection_params(zval *params, connection_policy_type type);
  virtual void write_host_to_params(zval *params);
  virtual void write_port_to_params(zval *params);
  virtual void write_socket_to_params(zval *params);
  virtual void set_srv(bool srv);
  virtual bool get_srv();
  virtual void set_dns(std::string dns);
  virtual std::string get_dns();
  virtual bool parse(std::string uri);
  virtual bool parse_username_password(std::string &usernamePassword);
  virtual bool parse_host_list(std::string &host_list);
};

#endif