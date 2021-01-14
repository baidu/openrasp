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

#include "dependency_writer.h"

namespace openrasp
{

void DependencyWriter::write_dependencys(const std::vector<std::string> &keys, const std::vector<DependencyItem> &deps, const std::string &source)
{
  json::json_pointer ptr = json::json_pointer(to_json_pointer(keys));
  json array = json::array();
  for (auto &dep : deps)
  {
    if (!dep.empty())
    {
      json item;
      item["path"] = dep.path;
      item["vendor"] = dep.vendor;
      item["product"] = dep.product;
      item["version"] = dep.version;
      item["source"] = source;
      array.push_back(item);
    }
  }
  j[ptr] = array;
}

} // namespace openrasp
