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

#ifndef _OPENRASP_UTILS_JSON_READER_H_
#define _OPENRASP_UTILS_JSON_READER_H_

#include "BaseReader.h"
#include "third_party/json/json.hpp"

namespace openrasp
{

using json = nlohmann::json;

class JsonReader : public BaseReader
{
private:
  json j;

public:
  JsonReader();
  JsonReader(const std::string &json_str);
  virtual std::string fetch_string(const std::vector<std::string> &keys, const std::string &default_value);
  virtual int64_t fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value);
  virtual bool fetch_bool(const std::vector<std::string> &keys, const bool &default_value);
  virtual std::vector<std::string> fetch_object_keys(const std::vector<std::string> &keys);
  virtual std::vector<std::string> fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value);
  virtual void load(const std::string &content);
  //write op
  void erase(const std::vector<std::string> &keys);
  void write_int64(const std::vector<std::string> &keys, const int64_t &value);
  void write_string(const std::vector<std::string> &keys, const std::string &value);

  void write_map_to_array(const std::vector<std::string> &keys, const std::string fkey, const std::string skey,
                          const std::map<std::string, std::string> &value);
  //Serialization
  std::string dump(const std::vector<std::string> &keys, bool pretty = false);
  std::string dump(bool pretty = false);
};

} // namespace openrasp

#endif
