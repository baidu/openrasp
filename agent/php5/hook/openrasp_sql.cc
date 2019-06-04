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

#include "openrasp_sql.h"
#include "openrasp_hook.h"
#include "openrasp_ini.h"
#include "openrasp_v8.h"
#include <string>
#include <map>
#include <set>
#include "agent/shared_config_manager.h"
#include "utils/utf.h"

extern "C"
{
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
#ifdef HAVE_LINE_COVERAGE
    void __gcov_flush();
#endif
}

static bool sql_policy_alarm(sql_connection_entry *conn_entry, sql_connection_entry::connection_policy_type policy_type, int enforce_policy)
{
    bool result = false;
    if (enforce_policy)
    {
        conn_entry->connection_entry_policy_log(policy_type);
        result = true;
    }
    else
    {
        if (slm != nullptr)
        {
            ulong connection_hash = conn_entry->build_hash_code(policy_type);
            long timestamp = (long)time(nullptr);
            if (!slm->log_exist(timestamp, connection_hash))
            {
                conn_entry->connection_entry_policy_log(policy_type);
            }
        }
    }
#ifdef HAVE_LINE_COVERAGE
    __gcov_flush();
#endif
    return result;
}

bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func, int enforce_policy)
{
    sql_connection_entry conn_entry;
    bool need_block = false;
    if (connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry))
    {
        if (conn_entry.check_high_privileged())
        {
            need_block = sql_policy_alarm(&conn_entry, sql_connection_entry::connection_policy_type::USER, enforce_policy);
        }
        if (conn_entry.check_weak_password())
        {
            need_block = sql_policy_alarm(&conn_entry, sql_connection_entry::connection_policy_type::PASSWORD, enforce_policy);
        }
    }
    return need_block;
}

void plugin_sql_check(char *query, int query_len, char *server TSRMLS_DC)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (isolate)
    {
        std::string cache_key = std::string(get_check_type_name(SQL)).append(query, query_len);
        if (OPENRASP_HOOK_G(lru).contains(cache_key))
        {
            return;
        }
        openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            params->Set(openrasp::NewV8String(isolate, "query"), openrasp::NewV8String(isolate, query, query_len));
            params->Set(openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, server));
            check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(SQL)), params, OPENRASP_CONFIG(plugin.timeout.millis));
        }
        if (check_result == openrasp::CheckResult::kCache)
        {
            OPENRASP_HOOK_G(lru).set(cache_key, true);
        }
        if (check_result == openrasp::CheckResult::kBlock)
        {
            handle_block(TSRMLS_C);
        }
    }
}

bool mysql_error_code_filtered(long err_code)
{
    static const std::set<long> mysql_error_codes = {
        1045,
        1060,
        1062,
        1064,
        1105,
        1367,
        1690};
    auto it = mysql_error_codes.find(err_code);
    if (it != mysql_error_codes.end())
    {
        return true;
    }
    return false;
}

void sql_query_error_alarm(char *server, char *query, const std::string &err_code, const std::string &err_msg TSRMLS_DC)
{
    zval *attack_params = nullptr;
    MAKE_STD_ZVAL(attack_params);
    array_init(attack_params);
    add_assoc_string(attack_params, "server", server, 1);
    add_assoc_string(attack_params, "query", query, 1);
    add_assoc_string(attack_params, "error_code", (char *)err_code.c_str(), 1);
    zval *plugin_message = nullptr;
    MAKE_STD_ZVAL(plugin_message);
    char *message_str = nullptr;
    std::string utf8_err_msg = openrasp::replace_invalid_utf8(err_msg);
    spprintf(&message_str, 0, _("%s error %s detected: %s."),
             server,
             err_code.c_str(),
             utf8_err_msg.c_str());
    ZVAL_STRING(plugin_message, message_str, 1);
    efree(message_str);
    OpenRASPActionType action = openrasp::scm->get_buildin_check_action(SQL_ERROR);
    openrasp_buildin_php_risk_handle(action, SQL_ERROR, 100, attack_params, plugin_message TSRMLS_CC);
}

void sql_connect_error_alarm(sql_connection_entry *sql_connection_p, const std::string &err_code, const std::string &err_msg TSRMLS_DC)
{
    zval *attack_params = nullptr;
    MAKE_STD_ZVAL(attack_params);
    array_init(attack_params);
    add_assoc_string(attack_params, "server", (char *)sql_connection_p->get_server().c_str(), 1);
    add_assoc_string(attack_params, "hostname", (char *)sql_connection_p->get_host().c_str(), 1);
    add_assoc_string(attack_params, "username", (char *)sql_connection_p->get_username().c_str(), 1);
    add_assoc_string(attack_params, "socket", (char *)sql_connection_p->get_socket().c_str(), 1);
    add_assoc_string(attack_params, "connectionString", (char *)sql_connection_p->get_connection_string().c_str(), 1);
    add_assoc_long(attack_params, "port", sql_connection_p->get_port());
    add_assoc_string(attack_params, "error_code", (char *)err_code.c_str(), 1);
    zval *plugin_message = nullptr;
    MAKE_STD_ZVAL(plugin_message);
    char *message_str = nullptr;
    std::string utf8_err_msg = openrasp::replace_invalid_utf8(err_msg);
    spprintf(&message_str, 0, _("%s error %s detected: %s."),
             sql_connection_p->get_server().c_str(),
             err_code.c_str(),
             utf8_err_msg.c_str());
    ZVAL_STRING(plugin_message, message_str, 1);
    efree(message_str);
    OpenRASPActionType action = openrasp::scm->get_buildin_check_action(SQL_ERROR);
    openrasp_buildin_php_risk_handle(action, SQL_ERROR, 100, attack_params, plugin_message TSRMLS_CC);
}
