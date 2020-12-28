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

#include "openrasp_content_type.h"
#include "utils/string.h"

namespace openrasp
{

const std::map<OpenRASPContentType::ContentType, std::vector<std::string>> OpenRASPContentType::type_map =
    {
        {OpenRASPContentType::ContentType::cApplicationForm, {"application/x-www-form-urlencoded"}},
        {OpenRASPContentType::ContentType::cApplicationJson, {"application/json"}},
        {OpenRASPContentType::ContentType::cApplicationXml, {"application/xml"}},
        {OpenRASPContentType::ContentType::cMultipartForm, {"multipart/form-data"}},
        {OpenRASPContentType::ContentType::cTextXml, {"text/xml"}},
        {OpenRASPContentType::ContentType::cTextHtml, {"text/html"}}};

OpenRASPContentType::ContentType OpenRASPContentType::classify_content_type(const std::string &content_type)
{
  for (auto iter = OpenRASPContentType::type_map.begin(); iter != OpenRASPContentType::type_map.end(); iter++)
  {
    std::vector<std::string> type_values = iter->second;
    for (std::string type : type_values)
    {
      if (content_type.find(type) != std::string::npos)
      {
        return iter->first;
      }
    }
  }
  return OpenRASPContentType::ContentType::cNull;
}

OpenRASPContentType::ContentType OpenRASPContentType::classify_accept(const std::string &accept)
{
  for (auto iter = OpenRASPContentType::type_map.begin(); iter != OpenRASPContentType::type_map.end(); iter++)
  {
    std::vector<std::string> type_values = iter->second;
    for (std::string type : type_values)
    {
      if (start_with(accept, type))
      {
        return iter->first;
      }
    }
  }
  return OpenRASPContentType::ContentType::cNull;
}

} // namespace openrasp