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

#include "utils/json.h"
#include "base_reader.h"
#include "third_party/json/json.hpp"

namespace openrasp
{

using json = nlohmann::json;

class JsonReader : public BaseReader
{
protected:
  json j;

public:
  JsonReader();
  JsonReader(const std::string &json_str);
  virtual std::string fetch_string(const std::vector<std::string> &keys, const std::string &default_value = "",
                                   const std::function<std::string(const std::string &value)> &validator = nullptr);
  virtual int64_t fetch_int64(const std::vector<std::string> &keys, const int64_t &default_value = 0,
                              const std::function<std::string(int64_t value)> &validator = nullptr);
  virtual bool fetch_bool(const std::vector<std::string> &keys, const bool &default_value = false);
  virtual std::vector<std::string> fetch_object_keys(const std::vector<std::string> &keys);
  virtual std::vector<std::string> fetch_strings(const std::vector<std::string> &keys, const std::vector<std::string> &default_value = std::vector<std::string>());
  virtual void load(const std::string &content);

  size_t get_array_size(const std::vector<std::string> &keys);
  //Serialization
  virtual std::string dump(const std::vector<std::string> &keys, bool pretty = false);
  virtual std::string dump(bool pretty = false);
  //write op
  void write_int64(const std::vector<std::string> &keys, const int64_t &value);
  void write_string(const std::vector<std::string> &keys, const std::string &value);
  void write_map(const std::vector<std::string> &keys, const std::map<std::string, std::string> &value);
  void write_map_to_array(const std::vector<std::string> &keys, const std::string fkey, const std::string skey,
                          const std::map<std::string, std::string> &value);
  void write_vector(const std::vector<std::string> &keys, const std::vector<std::string> &value);
  void write_int64_vector(const std::vector<std::string> &keys, const std::vector<int> &value);
  void update(const JsonReader &obj);
};

} // namespace openrasp

#endif
