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

#pragma once

#include "openrasp.h"
#include "openrasp_hook.h"
#include "utils/DoubleArrayTrie.h"
#include <string>
#include <unordered_set>

namespace openrasp
{
// #define DEFAULT_STRING_CONFIG_LENGTH (512)
#define WRITE_ARRAY_MAX_LENGTH (CHECK_TYPE_NR_ITEMS * 10 * 200 + 128 * 2)

class VariableConfigBlock
{
public:
  inline openrasp::DoubleArrayTrie::unit_t *get_check_type_white_array()
  {
    return check_type_white_array;
  }

  inline size_t get_white_array_size()
  {
    return white_array_size;
  }

  inline bool reset_white_array(const void *source, size_t num)
  {
    if (num > WRITE_ARRAY_MAX_LENGTH)
    {
      return false;
    }
    memset(&check_type_white_array, 0, sizeof(check_type_white_array));
    memcpy((void *)&check_type_white_array, source, num);
    white_array_size = num;
    return true;
  }

  inline long get_last_update_time()
  {
    return last_update_time;
  }

  inline long set_last_update_time(long last_update_time)
  {
    this->last_update_time = last_update_time;
  }

private:
  long last_update_time = 0;
  size_t white_array_size;
  openrasp::DoubleArrayTrie::unit_t check_type_white_array[WRITE_ARRAY_MAX_LENGTH + 1];
  // char clientip_header[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char block_redirect_url[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char block_content_json[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char block_content_xml[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char block_content_html[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char syslog_server_address[DEFAULT_STRING_CONFIG_LENGTH + 1];
  // char inject_html_urlprefix[DEFAULT_STRING_CONFIG_LENGTH + 1];

  // bool plugin_filter = true;
  // bool enforce_policy = false;
  // bool syslog_alarm_enable = false;

  // int log_maxburst = 100;
  // int syslog_facility;
  // int block_status_code = 302;
  // int syslog_connection_retry_interval = 200;

  // unsigned int timeout_ms = 100;
  // unsigned int log_maxstack = 10;
  // unsigned int log_max_backup = 30;
  // unsigned int plugin_maxstack = 100;
  // unsigned int slowquery_min_rows = 500;
  // unsigned int plugin_update_interval = 60;
  // unsigned int log_push_interval = 10;

  // long syslog_connection_timeout = 50;
  // long syslog_read_timeout = 10;
  // std::unordered_set<std::string> callable_blacklists;

  // inline void set_block_redirect_url(const char *block_redirect_url)
  // {
  //   strncpy(this->block_redirect_url, block_redirect_url, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_block_content_json(const char *block_content_json)
  // {
  //   strncpy(this->block_content_json, block_content_json, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_block_content_xml(const char *block_content_xml)
  // {
  //   strncpy(this->block_content_xml, block_content_xml, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_block_content_html(const char *block_content_html)
  // {
  //   strncpy(this->block_content_html, block_content_html, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_syslog_server_address(const char *syslog_server_address)
  // {
  //   strncpy(this->syslog_server_address, syslog_server_address, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_clientip_header(const char *clientip_header)
  // {
  //   strncpy(this->clientip_header, clientip_header, DEFAULT_STRING_CONFIG_LENGTH);
  // }

  // inline void set_inject_html_urlprefix(const char *inject_html_urlprefix)
  // {
  //   strncpy(this->inject_html_urlprefix, inject_html_urlprefix, DEFAULT_STRING_CONFIG_LENGTH);
  // }
};

} // namespace openrasp
