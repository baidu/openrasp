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

static void security_check(bool flag, int id, const char *msg TSRMLS_DC);
#define SECURITY_CHECK(flag, id, msg) security_check(flag, id, msg TSRMLS_CC)
#define STRTOBOOL strtobool

PHP_MINIT_FUNCTION(openrasp_security_policy)
{
    SECURITY_CHECK(PG(allow_url_include) == 0, 4001, _("allow_url_include should be turned off to prevent remote file inclusion vulnerability"));
    SECURITY_CHECK(PG(expose_php) == 0, 4002, _("expose_php should be turned off to prevent PHP version information disclosure"));
    SECURITY_CHECK(PG(display_errors) == 0, 4003, _("display_errors should be turned off to insulate your code from probing"));
    if (INI_STR("yaml.decode_php"))
    {
        SECURITY_CHECK(STRTOBOOL(INI_STR("yaml.decode_php"), strlen(INI_STR("yaml.decode_php"))) == false, 4004,
                       _("yaml.decode_php should be turned off to prevent serialized php objects which have the explicit tag \"!php/object\" from being unserialized"));
    }
    return SUCCESS;
}

static void security_check(bool flag, int id, const char *msg TSRMLS_DC)
{
    if (!flag)
    {
        zval policy_id, message;
        INIT_ZVAL(policy_id);
        INIT_ZVAL(message);
        ZVAL_LONG(&policy_id, id);
        ZVAL_STRING(&message, msg, 0);

        zval result;
        INIT_ZVAL(result);
        ALLOC_HASHTABLE(Z_ARRVAL(result));
        zend_hash_init(Z_ARRVAL(result), 0, 0, 0, 0);
        Z_TYPE(result) = IS_ARRAY;
        add_assoc_zval(&result, "policy_id", &policy_id);
        zval *policy_params = nullptr;
        MAKE_STD_ZVAL(policy_params);
        array_init(policy_params);
        add_assoc_long(policy_params, "pid", getpid());
        add_assoc_string(policy_params, "sapi", const_cast<char *>(sapi_module.name ? sapi_module.name : ""), 1);
        add_stack_to_params(policy_params TSRMLS_CC);
        add_assoc_zval(&result, "policy_params", policy_params);
        add_assoc_zval(&result, "message", &message);
        LOG_G(policy_logger).log(LEVEL_INFO, &result TSRMLS_CC);
        zval_dtor(&result);
    }
}