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

void MongoConnectionEntry::append_host_port(const std::string &host, int port)
{
  hosts.push_back(host);
  ports.push_back(port);
}

void MongoConnectionEntry::write_host_to_params(zval *params)
{
  zval host_arr;
  array_init(&host_arr);
  for (auto host : hosts)
  {
    add_next_index_string(&host_arr, (char *)host.c_str());
  }
  add_assoc_zval(params, "hostnames", &host_arr);
}

void MongoConnectionEntry::write_port_to_params(zval *params)
{
  zval port_arr;
  array_init(&port_arr);
  for (int port : ports)
  {
    add_next_index_long(&port_arr, port);
  }
  add_assoc_zval(params, "ports", &port_arr);
}
