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

#ifndef OPENRASP_CONTENT_TYPE_H
#define OPENRASP_CONTENT_TYPE_H

#include <map>
#include <vector>

namespace openrasp
{

class OpenRASPContentType
{
public:
  enum ContentType
  {
    cApplicationForm,
    cApplicationJson,
    cApplicationXml,
    cMultipartForm,
    cTextXml,
    cTextHtml,
    cNull
  };

  static const std::map<ContentType, std::vector<std::string>> type_map;
  static ContentType classify_content_type(const std::string &content_type);
  static ContentType classify_accept(const std::string &accept);
};

} // namespace openrasp

#endif