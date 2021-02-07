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

#ifndef _OPENRASP_UTILS_NET_H_
#define _OPENRASP_UTILS_NET_H_

#include <string>
#include <vector>
#include <map>

namespace openrasp
{

void fetch_if_addrs(std::map<std::string, std::string> &if_addr_map);
void fetch_hw_addrs(std::vector<std::string> &hw_addrs);
bool fetch_source_in_ip_packets(char *local_ip, size_t len, char *url);
std::vector<std::string> lookup_host(const std::string &host);

} // namespace openrasp

#endif
