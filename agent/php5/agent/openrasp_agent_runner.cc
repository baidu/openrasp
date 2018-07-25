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

#include "openrasp.h"
#include "openrasp_ini.h"
#include "openrasp_agent_runner.h"
#include "openrasp_agent_manager.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>

extern "C"
{
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
}

#define DEFAULT_PLUGIN_AUTORELOAD_INTERVAL 10
#define DEFAULT_LOG_AUTORELOAD_INTERVAL 10

namespace openrasp
{
volatile static int signal_received = 0;
static void signal_handler(int signal_no)
{
  signal_received = signal_no;
}

static void install_signal_handler()
{
  struct sigaction sa_usr = {0};
  sa_usr.sa_flags = 0;
  sa_usr.sa_handler = signal_handler;
  sigaction(SIGTERM, &sa_usr, NULL);
}

static void agent_exit()
{
  exit(0);
}

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
  data->append((char *)ptr, size * nmemb);
  return size * nmemb;
}

BaseAgentRunner::BaseAgentRunner(int autoreload_interval) : _autoreload_interval(autoreload_interval)
{
}

PluginAgentRunner::PluginAgentRunner() : BaseAgentRunner(DEFAULT_PLUGIN_AUTORELOAD_INTERVAL)
{
}

void PluginAgentRunner::run()
{
  OPENRASP_SET_PROC_NAME("plugin-agent");
  install_signal_handler();
  std::string _root_dir(openrasp_ini.root_dir);
  while (true)
  {
    for (int i = 0; i < _autoreload_interval; ++i)
    {
      sleep(1);
      if (signal_received == SIGTERM)
      {
        agent_exit();
      }
    }
    auto curl = curl_easy_init();
    if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_URL, "http://cq02-scloud-csiemtest.cq02.baidu.com:8888/test.php");
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
      curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

      std::string response_string;
      std::string header_string;
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
      curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

      char *url;
      long response_code;
      double elapsed;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
      curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

      curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      curl = NULL;
      zval *return_value = nullptr;
      MAKE_STD_ZVAL(return_value);
      php_json_decode(return_value, (char *)response_string.c_str(), response_string.size(), 1, 512 TSRMLS_CC);
      zval **origin_zv;
      if (Z_TYPE_P(return_value) == IS_ARRAY &&
          zend_hash_find(Z_ARRVAL_P(return_value), "content", strlen("content") + 1, (void **)&origin_zv) == SUCCESS &&
          Z_TYPE_PP(origin_zv) == IS_STRING)
      {
        std::ofstream out_file(_root_dir + "/plugins/offical.js", std::ofstream::in | std::ofstream::out | std::ofstream::trunc);
        if (out_file.good())
        {
          out_file << Z_STRVAL_PP(origin_zv);
          out_file.close();
        }
      }
      zval_ptr_dtor(&return_value);
    }
  }
};

LogAgentRunner::LogAgentRunner() : BaseAgentRunner(DEFAULT_LOG_AUTORELOAD_INTERVAL)
{
}

void LogAgentRunner::run()
{
  OPENRASP_SET_PROC_NAME("log-agent");
  install_signal_handler();

  while (true)
  {
    for (int i = 0; i < _autoreload_interval; ++i)
    {
      sleep(1);
      if (signal_received == SIGTERM)
      {
        agent_exit();
      }
    }
    //TODO
  }
};

} // namespace openrasp
