/*
 * Copyright 2017-2021 Baidu Inc.
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

ZEND_INI_MH(OnUpdateOpenraspCString);
ZEND_INI_MH(OnUpdateOpenraspBool);
ZEND_INI_MH(OnUpdateOpenraspHeartbeatInterval);

class Openrasp_ini
{
public:
  char *root_dir;
  char *locale;
  unsigned int heartbeat_interval = 90;
  char *backend_url;
  char *app_id;
  char *app_secret;
  char *rasp_id;
  bool remote_management_enable = true;
  bool ssl_verifypeer = false;
  bool iast_enable = false;

  static const char *APPID_REGEX;
  static const char *APPSECRET_REGEX;
  static const char* RASPID_REGEX;
  bool verify_remote_management_ini(std::string &error);
  bool verify_rasp_id();
};

extern Openrasp_ini openrasp_ini;

bool strtobool(const char *str, int len);