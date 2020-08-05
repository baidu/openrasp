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

#ifndef _OPENRASP_UTILS_JSON_H_
#define _OPENRASP_UTILS_JSON_H_

#include <string>
#include <vector>
#include "string.h"

namespace openrasp
{

inline const std::string to_json_pointer(const std::vector<std::string> &keys)
{
  std::string pointer_str;
  for (std::string key : keys)
  {
    string_replace(key, "/", "~1"); //https://tools.ietf.org/html/rfc6901
    pointer_str.push_back('/');
    pointer_str.append(key);
  }
  return pointer_str;
}

} // namespace openrasp

#endif
