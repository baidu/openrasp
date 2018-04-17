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

#include "openrasp_ini.h"
#include <regex>
#include <limits>

Openrasp_ini openrasp_ini;

ZEND_INI_MH(OnUpdateOpenraspIntGEZero)
{
    long tmp = zend_atol(new_value, new_value_length);
    if (tmp < 0 || tmp > std::numeric_limits<unsigned int>::max())
    {
        return FAILURE;
    }
    *reinterpret_cast<int *>(mh_arg1) = tmp;
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspCString)
{
    *reinterpret_cast<char **>(mh_arg1) = new_value_length ? new_value : nullptr;
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspBool)
{
    bool *tmp = reinterpret_cast<bool *>(mh_arg1);
    if (new_value_length == 2 && strcasecmp("on", new_value) == 0)
    {
        *tmp = true;
    }
    else if (new_value_length == 3 && strcasecmp("yes", new_value) == 0)
    {
        *tmp = true;
    }
    else if (new_value_length == 4 && strcasecmp("true", new_value) == 0)
    {
        *tmp = true;
    }
    else
    {
        *tmp = atoi(new_value);
    }
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspSet)
{
    std::unordered_set<std::string> *p = reinterpret_cast<std::unordered_set<std::string> *>(mh_arg1);
    p->clear();
    if (new_value)
    {
        std::regex re(R"([\s,]+)");
        const std::cregex_token_iterator end;
        for (std::cregex_token_iterator it(new_value, new_value + new_value_length, re, -1); it != end; it++)
        {
            p->insert(it->str());
        }
    }
    return SUCCESS;
}