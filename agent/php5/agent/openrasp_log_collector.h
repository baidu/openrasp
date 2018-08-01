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

#ifndef _OPENRASP_LOG_COLLECTOR_
#define _OPENRASP_LOG_COLLECTOR_

#include <string>
#include <thread>
#include "openrasp_log.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "utils/curl_helper.h"

#include <sys/inotify.h>
#include <sys/epoll.h>
#include "readerwriterqueue.h"

namespace openrasp
{

class LogDirEntry
{
public:
  const std::string backend_url;
  const std::string dir_abs_path;
  const std::string prefix;
  std::string active_file_name;
  int active_file_fd = 0;
  std::thread active_thread;
  moodycamel::ReaderWriterQueue<char> log_queue{1024};

  LogDirEntry(const std::string dir_abs_path, const std::string prefix, const std::string backend_url)
      : dir_abs_path(dir_abs_path), prefix(prefix), backend_url(backend_url)
  {
  }
};

class LogCollector
{
public:
  LogCollector();
  void run();

protected:
  void process_log_push(int dir_watch_fd);
  void add_watch_file(std::string filename, bool newly_created, int dir_watch_fd);
  void update_formatted_date_suffix();
  void post_logs_via_curl(zval *log_arr, CURL *curl, std::string url_string);

private:
  bool _exit = false;
  int inotify_instance, epoll_instance;
  struct epoll_event _ev;
  struct stat sb;
  std::string _formatted_date_suffix;
  std::string _root_dir;
  std::string _backend;
  std::string _default_slash;
  std::unordered_map<int, LogDirEntry *> log_dir_umap;
  std::unordered_map<int, int> log_file_umap;
  LogCollector(const LogCollector &) = delete;
  LogCollector &operator=(const LogCollector &) = delete;
  std::vector<LogDirEntry> log_dirs;
};

} // namespace openrasp

#endif
