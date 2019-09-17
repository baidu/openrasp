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

static bool sql_policy_alarm(sql_connection_entry *conn_entry, sql_connection_entry::connection_policy_type policy_type)
{
    bool result = false;
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
        else
        {
            conn_entry->connection_entry_policy_log(policy_type);
        }
        }
#ifdef HAVE_LINE_COVERAGE
    __gcov_flush();
#endif
    return result;
}

bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func)
{
    sql_connection_entry conn_entry;
    bool need_block = false;
    if (connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry))
    {
        if (conn_entry.check_high_privileged())
        {
            need_block = sql_policy_alarm(&conn_entry, sql_connection_entry::connection_policy_type::USER);
        }
        if (conn_entry.check_weak_password())
        {
            need_block = sql_policy_alarm(&conn_entry, sql_connection_entry::connection_policy_type::PASSWORD);
        }
    }
    return need_block;
}

void plugin_sql_check(char *query, int query_len, const char *server)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (query && strlen(query) == query_len && isolate)
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
            handle_block();
        }
    }
}

bool is_mysql_error_code_monitored(long err_code)
{
    if (openrasp::scm->sql_error_code_exist(err_code))
    {
        return true;
    }
    return false;
}

void sql_query_error_alarm(char *server, char *query, const std::string &err_code, const std::string &err_msg)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (isolate && server && query)
    {
        openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            params->Set(openrasp::NewV8String(isolate, "query"), openrasp::NewV8String(isolate, query, strlen(query)));
            params->Set(openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, server));
            params->Set(openrasp::NewV8String(isolate, "error_code"), openrasp::NewV8String(isolate, err_code));
            std::string utf8_err_msg = openrasp::replace_invalid_utf8(err_msg);
            params->Set(openrasp::NewV8String(isolate, "error_msg"), openrasp::NewV8String(isolate, utf8_err_msg));
            check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(SQL_ERROR)), params, OPENRASP_CONFIG(plugin.timeout.millis));
        }
        if (check_result == openrasp::CheckResult::kBlock)
        {
            handle_block();
        }
    }
}

void sql_connect_error_alarm(sql_connection_entry *sql_connection_p, const std::string &err_code, const std::string &err_msg)
{
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (isolate && sql_connection_p)
    {
        openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            params->Set(openrasp::NewV8String(isolate, "server"), openrasp::NewV8String(isolate, sql_connection_p->get_server()));
            params->Set(openrasp::NewV8String(isolate, "hostname"), openrasp::NewV8String(isolate, sql_connection_p->get_host()));
            params->Set(openrasp::NewV8String(isolate, "username"), openrasp::NewV8String(isolate, sql_connection_p->get_username()));
            params->Set(openrasp::NewV8String(isolate, "socket"), openrasp::NewV8String(isolate, sql_connection_p->get_socket()));
            params->Set(openrasp::NewV8String(isolate, "connectionString"), openrasp::NewV8String(isolate, sql_connection_p->get_connection_string()));
            params->Set(openrasp::NewV8String(isolate, "port"), v8::Integer::New(isolate, sql_connection_p->get_port()));
            params->Set(openrasp::NewV8String(isolate, "error_code"), openrasp::NewV8String(isolate, err_code));
            std::string utf8_err_msg = openrasp::replace_invalid_utf8(err_msg);
            params->Set(openrasp::NewV8String(isolate, "error_msg"), openrasp::NewV8String(isolate, utf8_err_msg));
            check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(SQL_ERROR)), params, OPENRASP_CONFIG(plugin.timeout.millis));
        }
        if (check_result == openrasp::CheckResult::kBlock)
        {
            handle_block(TSRMLS_C);
        }
    }
}
