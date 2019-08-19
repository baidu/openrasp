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

#include "openrasp_security_policy.h"
#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include <string>
static void security_check(bool flag, int id, const char *msg);
#define SECURITY_CHECK(flag, id, msg) security_check(flag, id, msg)
#define STRTOBOOL strtobool

PHP_MINIT_FUNCTION(openrasp_security_policy)
{
    SECURITY_CHECK(PG(allow_url_include) == 0, 4001, _("allow_url_include should be turned off to prevent File Inclusion vulnerability"));
    SECURITY_CHECK(PG(expose_php) == 0, 4002, _("expose_php should be turned off to prevent PHP Version information disclosure"));
    SECURITY_CHECK(PG(display_errors) == 0, 4003, _("display_errors should be turned off to prevent printing errors to the screen"));
    if (INI_STR("yaml.decode_php"))
    {
        SECURITY_CHECK(STRTOBOOL(INI_STR("yaml.decode_php"), strlen(INI_STR("yaml.decode_php"))) == false, 4004, _("yaml.decode_php should be turned off to prevent serializing php objects"));
    }
    return SUCCESS;
}

static void security_check(bool flag, int id, const char *msg)
{
    if (!flag)
    {
        zval result;
        array_init(&result);
        add_assoc_long(&result, "policy_id", id);
        zval policy_params;
        array_init(&policy_params);
        add_assoc_long(&policy_params, "pid", getpid());
        add_assoc_string(&policy_params, "sapi", const_cast<char *>(sapi_module.name ? sapi_module.name : ""));
        add_stack_to_params(&policy_params);
        add_assoc_zval(&result, "policy_params", &policy_params);
        add_assoc_string(&result, "message", const_cast<char *>(msg));
        LOG_G(policy_logger).log(LEVEL_INFO, &result);
        zval_dtor(&result);
    }
}