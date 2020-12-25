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

#include "openrasp_sql.h"
#include "openrasp_hook.h"
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include "openrasp_v8.h"
#include <string>
#include <map>
#include "agent/shared_config_manager.h"
#include "utils/utf.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/sql_object.h"
#include "hook/data/sql_username_object.h"
#include "hook/data/sql_password_object.h"
#include "hook/checker/policy_detector.h"

extern "C"
{
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
#ifdef HAVE_LINE_COVERAGE
    void __gcov_flush();
#endif
}

void sql_connection_policy_check(INTERNAL_FUNCTION_PARAMETERS, init_sql_connection_t connection_init_func, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    if (connection_init_func)
    {
        if (connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_obj))
        {
            openrasp::data::SqlUsernameObject suo(sql_connection_obj);
            openrasp::checker::PolicyDetector username_detector(suo);
            username_detector.run();
            openrasp::data::SqlPasswordObject spo(sql_connection_obj);
            openrasp::checker::PolicyDetector weak_passwd_detector(spo);
            weak_passwd_detector.run();
        }
    }
}

void plugin_sql_check(zval *query, const std::string &server)
{
    openrasp::data::SqlObject sql_obj(server, query);
    openrasp::checker::V8Detector v8_detector(sql_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}
