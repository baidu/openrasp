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

#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include <limits>
#include "utils/string.h"
#include "utils/regex.h"

Openrasp_ini openrasp_ini;

static const int MIN_HEARTBEAT_INTERVAL = 10;
const char *Openrasp_ini::APPID_REGEX = "^[0-9a-fA-F]{40}$";
const char *Openrasp_ini::APPSECRET_REGEX = "^[0-9a-zA-Z_-]{43,45}$";
const char *Openrasp_ini::RASPID_REGEX = "^[0-9a-zA-Z]{16,512}$";

bool Openrasp_ini::verify_remote_management_ini(std::string &error)
{
    if (openrasp::empty(backend_url))
    {
        error = std::string(_("openrasp.backend_url is required when remote management is enabled."));
        return false;
    }
    if (openrasp::empty(app_id))
    {
        error = std::string(_("openrasp.app_id is required when remote management is enabled."));
        return false;
    }
    else
    {
        if (!openrasp::regex_match(app_id, Openrasp_ini::APPID_REGEX))
        {
            error = std::string(_("openrasp.app_id must be exactly 40 characters long."));
            return false;
        }
    }
    if (openrasp::empty(app_secret))
    {
        error = std::string(_("openrasp.app_secret is required when remote management is enabled."));
        return false;
    }
    else
    {
        if (!openrasp::regex_match(app_secret, Openrasp_ini::APPSECRET_REGEX))
        {
            error = std::string(_("openrasp.app_secret configuration format is incorrect."));
            return false;
        }
    }
    return true;
}

bool Openrasp_ini::verify_rasp_id()
{
    if (!openrasp::empty(rasp_id))
    {
        return openrasp::regex_match(rasp_id, Openrasp_ini::RASPID_REGEX);
    }
    return true;
}

ZEND_INI_MH(OnUpdateOpenraspCString)
{
    *reinterpret_cast<char **>(mh_arg1) = new_value->val;
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspBool)
{
    bool *tmp = reinterpret_cast<bool *>(mh_arg1);
    *tmp = strtobool(new_value->val, new_value->len);
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspHeartbeatInterval)
{
    long tmp = zend_atol(new_value->val, new_value->len);
    if (tmp < MIN_HEARTBEAT_INTERVAL || tmp > 1800)
    {
        return FAILURE;
    }
    *reinterpret_cast<int *>(mh_arg1) = tmp;
    return SUCCESS;
}

bool strtobool(const char *str, int len)
{
    return atoi(str);
}