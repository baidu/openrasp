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

#include "safe_shutdown_manager.h"
#include <vector>
#include "openrasp_utils.h"
#include <fstream>
#include <sstream>

extern "C"
{
#include "php_main.h"
}
namespace openrasp
{
std::unique_ptr<SafeShutDownManager> ssdm = nullptr;

SafeShutDownManager::SafeShutDownManager(ShmManager *mm)
    : BaseManager(mm),
      master_pid(nullptr)
{
}

bool SafeShutDownManager::startup()
{
  init_process_pid = getpid();
  char *shm_block = shm_manager->create(SHMEM_SEC_MASTER_PID, sizeof(pid_t));
  if (!shm_block)
  {
    return false;
  }
  memset(shm_block, 0, sizeof(pid_t));
  master_pid = reinterpret_cast<pid_t *>(shm_block);
  *master_pid = init_process_pid;
  initialized = true;
  return true;
}

bool SafeShutDownManager::shutdown()
{
  if (initialized)
  {
    if (strcmp(sapi_module.name, "fpm-fcgi") == 0)
    {
      pid_t fpm_master_pid = search_fpm_master_pid();
      if (fpm_master_pid)
      {
        *master_pid = fpm_master_pid;
      }
    }
    if (*master_pid && !is_master_current_process())
    {
      return true;
    }
    master_pid = nullptr;
    shm_manager->destroy(SHMEM_SEC_MASTER_PID);
    initialized = false;
  }
}

bool SafeShutDownManager::is_master_current_process()
{
  return (getpid() == *master_pid);
}

pid_t SafeShutDownManager::get_master_pid()
{
  return *master_pid;
}

pid_t SafeShutDownManager::search_fpm_master_pid()
{
  TSRMLS_FETCH();
  std::vector<std::string> processes;
  openrasp_scandir("/proc", processes,
                   [](const char *filename) {
                     TSRMLS_FETCH();
                     struct stat sb;
                     if (VCWD_STAT(("/proc/" + std::string(filename)).c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
                     {
                       return true;
                     }
                     return false;
                   });
  for (std::string pid : processes)
  {
    std::string stat_file_path = "/proc/" + pid + "/stat";
    if (VCWD_ACCESS(stat_file_path.c_str(), F_OK) == 0)
    {
      std::ifstream ifs_stat(stat_file_path);
      std::string stat_line;
      std::getline(ifs_stat, stat_line);
      if (stat_line.empty())
      {
        continue;
      }
      std::stringstream stat_items(stat_line);
      std::string item;
      for (int i = 0; i < 4; ++i)
      {
        std::getline(stat_items, item, ' ');
      }
      int ppid = std::atoi(item.c_str());
      if (ppid == init_process_pid)
      {
        std::string cmdline_file_path = "/proc/" + pid + "/cmdline";
        std::ifstream ifs_cmdline(cmdline_file_path);
        std::string cmdline;
        std::getline(ifs_cmdline, cmdline);
        if (cmdline.find("php-fpm: master process") == 0)
        {
          return std::atoi(pid.c_str());
        }
      }
    }
  }
  return 0;
}

} // namespace openrasp
