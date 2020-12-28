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
        openrasp::JsonReader base_json;
        base_json.write_int64({"policy_id"}, id);
        base_json.write_int64({"policy_params", "pid"}, getpid());
        base_json.write_string({"policy_params", "sapi"}, (sapi_module.name ? sapi_module.name : ""));
        base_json.write_vector({"policy_params", "stack"}, format_debug_backtrace_arr());
        base_json.write_string({"message"}, msg);
        if (!base_json.has_error())
        {
            LOG_G(policy_logger).log(LEVEL_INFO, base_json);
        }
    }
}