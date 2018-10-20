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

#ifndef _LOG_COLLECT_ITEM_H_
#define _LOG_COLLECT_ITEM_H_

#include "openrasp.h"
#include <fstream>
#include <memory>

namespace openrasp
{

class LogCollectItem
{
public:
  static const long time_offset;

private:
  static const std::string status_file;
  const std::string name;
  const std::string url_path;
  std::string curr_suffix;

  std::ifstream ifs;
  int fpos = 0;
  long st_ino = 0;
  long last_post_time = 0;

public:
  LogCollectItem(const std::string name, const std::string url_path TSRMLS_DC);
  inline void update_curr_suffix();
  std::string get_active_log_file();
  void determine_fpos(TSRMLS_D);
  std::string get_cpmplete_url();
  std::string get_post_logs();
  void update_status();
  bool need_rotate();
  void handle_rotate(bool need_rotate TSRMLS_DC);

private:
  void open_active_log();
  inline std::string get_base_dir_path();
  void save_status_snapshot();
  void clear();
  void cleanup_expired_logs(TSRMLS_D);
};

} // namespace openrasp

#endif
