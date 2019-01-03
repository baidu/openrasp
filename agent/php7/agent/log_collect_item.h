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

#ifndef _LOG_COLLECT_ITEM_H_
#define _LOG_COLLECT_ITEM_H_

#include "openrasp.h"
#include <fstream>
#include <memory>
#include <map>

namespace openrasp
{

class LogCollectItem
{
public:
  static const long time_offset;
  static const std::map<int, const std::string> instance_url_map;
  static const std::map<int, const std::string> instance_name_map;

public:
  LogCollectItem(int instance_id, bool collect_enable);

  bool has_error() const;
  void update_fpos();
  void update_last_post_time();
  void determine_fpos();
  inline void update_curr_suffix();
  void save_status_snapshot() const;

  bool need_rotate() const;
  void handle_rotate(bool need_rotate);

  bool get_post_logs(std::string &body);
  bool get_collect_enable() const;
  std::string get_cpmplete_url() const;
  std::string get_active_log_file() const;

private:
  static const std::string status_file;
  const int instance_id;
  std::string curr_suffix;
  std::ifstream ifs;
  int fpos = 0;
  long st_ino = 0;
  long last_post_time = 0;
  bool collect_enable = false;
  bool error = false;

private:
  void clear();
  void open_active_log();
  void cleanup_expired_logs() const;
  inline std::string get_base_dir_path() const;
  long get_active_file_inode();
  bool log_content_qualified(const std::string &content);

  bool is_instance_valid() const;
  const std::string get_name() const;
  const std::string get_url_path() const;
};

} // namespace openrasp

#endif
