/*
 * Copyright 2017-2020 Baidu Inc.
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
#include "hook/data/sql_error_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/sql_object.h"

extern "C"
{
#include "zend_ini.h"
}

POST_HOOK_FUNCTION(mysql_connect, DB_CONNECTION);
POST_HOOK_FUNCTION(mysql_connect, SQL_ERROR);
POST_HOOK_FUNCTION(mysql_pconnect, DB_CONNECTION);
POST_HOOK_FUNCTION(mysql_pconnect, SQL_ERROR);
PRE_HOOK_FUNCTION(mysql_query, SQL);
POST_HOOK_FUNCTION(mysql_query, SQL_ERROR);
PRE_HOOK_FUNCTION(mysql_db_query, SQL);
POST_HOOK_FUNCTION(mysql_db_query, SQL_ERROR);
PRE_HOOK_FUNCTION(mysql_unbuffered_query, SQL);
POST_HOOK_FUNCTION(mysql_unbuffered_query, SQL_ERROR);

static long fetch_mysql_errno(uint32_t param_count, zval *params[] TSRMLS_DC);
static std::string fetch_mysql_error(uint32_t param_count, zval *params[] TSRMLS_DC);

static bool init_mysql_connection_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj, int persistent)
{
    char *user = nullptr, *passwd = nullptr, *host_and_port = nullptr, *socket = nullptr, *host = nullptr, *tmp = nullptr;
    int user_len = 0, passwd_len = 0, host_len = 0, port = MYSQL_PORT;
    long client_flags = 0;
    zend_bool new_link = 0;
    static char *default_host = INI_STR("mysql.default_host");
    static char *default_user = INI_STR("mysql.default_user");
    static char *default_password = INI_STR("mysql.default_password");
    static char *default_socket = INI_STR("mysql.default_socket");
    static long default_port = INI_INT("mysql.default_port");

    if (default_port <= 0)
    {
        default_port = MYSQL_PORT;
    }
    if (PG(sql_safe_mode))
    {
        if (ZEND_NUM_ARGS() > 0)
        {
            return false;
        }
        host_and_port = passwd = nullptr;
#if (PHP_MINOR_VERSION == 3)
        user = php_get_current_user();
#else
        user = php_get_current_user(TSRMLS_C);
#endif
    }
    else
    {
        if (persistent)
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!s!s!l", &host_and_port, &host_len,
                                      &user, &user_len, &passwd, &passwd_len,
                                      &client_flags) == FAILURE)
            {
                return false;
            }
        }
        else
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!s!s!bl", &host_and_port, &host_len,
                                      &user, &user_len, &passwd, &passwd_len,
                                      &new_link, &client_flags) == FAILURE)
            {
                return false;
            }
        }
        if (!host_and_port)
        {
            host_and_port = default_host;
        }
        if (!user)
        {
            user = default_user;
        }
        if (!passwd)
        {
            passwd = default_password;
        }
    }
    sql_connection_obj.set_server("mysql");
    socket = default_socket;
    bool using_socket = false;

    if (host_and_port && (tmp = strchr(host_and_port, ':')))
    {
        std::string host_str = std::string(host_and_port, tmp - host_and_port);
        tmp++;
        if (tmp[0] != '/')
        {
            port = atoi(tmp);
            if ((tmp = strchr(tmp, ':')))
            {
                tmp++;
                socket = tmp;
                using_socket = true;
            }
            else
            {
                sql_connection_obj.set_host(host_str);
                sql_connection_obj.set_port(port);
            }
        }
        else
        {
            socket = tmp;
            using_socket = true;
        }
        using_socket = ("localhost" == host_str);
    }
    else
    {
        using_socket = (host_and_port == nullptr || strncmp(host_and_port, "localhost", strlen("localhost")) == 0);
        sql_connection_obj.set_host(SAFE_STRING(host_and_port));
        sql_connection_obj.set_port(default_port);
    }
    sql_connection_obj.set_using_socket(using_socket);
    sql_connection_obj.set_socket(SAFE_STRING(socket));
    sql_connection_obj.set_username(SAFE_STRING(user));
    sql_connection_obj.set_password(SAFE_STRING(passwd));
    return true;
}

static inline bool init_mysql_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    return init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_obj, 0);
}

static inline bool init_mysql_pconnect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    return init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_obj, 1);
}

static void mysql_connect_error_intercept(INTERNAL_FUNCTION_PARAMETERS, init_sql_connection_t connection_init_func)
{
    long error_code = fetch_mysql_errno(0, nullptr TSRMLS_CC);
    std::string error_msg = fetch_mysql_error(0, nullptr TSRMLS_CC);
    openrasp::data::SqlConnectionObject sco;
    connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, sco);
    openrasp::data::SqlErrorObject seo(sco, "mysql", error_code, error_msg);
    openrasp::checker::V8Detector error_checker(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    error_checker.run();
}

//mysql_connect
void post_global_mysql_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        openrasp::data::SqlConnectionObject sco;
        sql_connection_policy_check(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_connect_conn_entry, sco);
    }
}

void post_global_mysql_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        mysql_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_connect_conn_entry);
    }
}

//mysql_pconnect
void post_global_mysql_pconnect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        openrasp::data::SqlConnectionObject sco;
        sql_connection_policy_check(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_pconnect_conn_entry, sco);
    }
}

void post_global_mysql_pconnect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        mysql_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_pconnect_conn_entry);
    }
}

//mysql_query
void pre_global_mysql_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *query = nullptr;
    zval *mysql_link = nullptr;

    if (UNLIKELY(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|r", &query, &mysql_link) == FAILURE))
    {
        return;
    }

    plugin_sql_check(query, "mysql" TSRMLS_CC);
}

void post_global_mysql_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        zval *query = nullptr;
        zval *mysql_link = nullptr;
        if (UNLIKELY(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|r", &query, &mysql_link) == FAILURE))
        {
            return;
        }
        zval *args[1];
        int param_num = 0;
        if (nullptr != mysql_link)
        {
            args[0] = mysql_link;
            param_num = 1;
        }
        long error_code = fetch_mysql_errno(param_num, args TSRMLS_CC);
        std::string error_msg = fetch_mysql_error(param_num, args TSRMLS_CC);
        openrasp::data::SqlErrorObject seo(openrasp::data::SqlObject("mysql", query), "mysql", error_code, error_msg);
        openrasp::checker::V8Detector v8_detector(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
        v8_detector.run();
    }
}

static long fetch_mysql_errno(uint32_t param_count, zval *params[] TSRMLS_DC)
{
    long error_code = 0;
    zval function_name, retval;
    INIT_ZVAL(function_name);
    ZVAL_STRING(&function_name, "mysql_errno", 0);
    if (call_user_function(EG(function_table), nullptr, &function_name, &retval, param_count, params TSRMLS_CC) == SUCCESS &&
        Z_TYPE(retval) == IS_LONG)
    {
        error_code = Z_LVAL(retval);
    }
    return error_code;
}

static std::string fetch_mysql_error(uint32_t param_count, zval *params[] TSRMLS_DC)
{
    std::string error_msg;
    zval function_name, retval;
    INIT_ZVAL(function_name);
    ZVAL_STRING(&function_name, "mysql_error", 0);
    if (call_user_function(EG(function_table), nullptr, &function_name, &retval, param_count, params TSRMLS_CC) == SUCCESS)
    {
        if (Z_TYPE(retval) == IS_STRING)
        {
            error_msg = std::string(Z_STRVAL(retval));
        }
        zval_dtor(&retval);
    }
    return error_msg;
}

void pre_global_mysql_db_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *db = nullptr;
    zval *query = nullptr;
    zval *mysql_link = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|r", &db, &query, &mysql_link) == FAILURE)
    {
        return;
    }

    plugin_sql_check(query, "mysql" TSRMLS_CC);
}

void post_global_mysql_db_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        zval *db = nullptr;
        zval *query = nullptr;
        zval *mysql_link = nullptr;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|r", &db, &query, &mysql_link) == FAILURE)
        {
            return;
        }
        zval *args[1];
        int param_num = 0;
        if (nullptr != mysql_link)
        {
            args[0] = mysql_link;
            param_num = 1;
        }
        long error_code = fetch_mysql_errno(param_num, args TSRMLS_CC);
        std::string error_msg = fetch_mysql_error(param_num, args TSRMLS_CC);
        openrasp::data::SqlErrorObject seo(openrasp::data::SqlObject("mysql", query), "mysql", error_code, error_msg);
        openrasp::checker::V8Detector v8_detector(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
        v8_detector.run();
    }
}

void pre_global_mysql_unbuffered_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_mysql_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void post_global_mysql_unbuffered_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_mysql_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}