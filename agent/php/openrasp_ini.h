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
#include <string>
#include <unordered_set>

ZEND_INI_MH(OnUpdateOpenraspDoubleGEZero);
ZEND_INI_MH(OnUpdateOpenraspIntGEZero);
ZEND_INI_MH(OnUpdateOpenraspCString);
ZEND_INI_MH(OnUpdateOpenraspBool);
ZEND_INI_MH(OnUpdateOpenraspSet);

class Openrasp_ini
{
  public:
    char *root_dir;
    char *locale;
    unsigned int timeout_ms = 100;
    char *syslog_server_address;
    int log_maxburst = 1000;
    int syslog_facility;
    bool syslog_alarm_enable = 0;
    long syslog_connection_timeout = 50;
    long syslog_read_timeout = 10;
    int syslog_connection_retry_interval = 200;
    int block_status_code = 302;
    char *inject_html_urlprefix;
    unsigned int slowquery_min_rows = 500;
    unsigned int plugin_maxstack = 100;
    unsigned int log_maxstack = 10;
    bool enforce_policy = false;
    char *block_url;
    std::unordered_set<std::string> hooks_ignore;
    std::unordered_set<std::string> callable_blacklists;
};

extern Openrasp_ini openrasp_ini;

bool strtobool(const char *str, int len);