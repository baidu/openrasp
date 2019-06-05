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

extern "C"
{
#include "zend_ini.h"
}
/**
 * mysqli相关hook点
 */
POST_HOOK_FUNCTION(mysqli_connect, DB_CONNECTION);
POST_HOOK_FUNCTION(mysqli_connect, SQL_ERROR);
POST_HOOK_FUNCTION(mysqli_real_connect, DB_CONNECTION);
POST_HOOK_FUNCTION(mysqli_real_connect, SQL_ERROR);
PRE_HOOK_FUNCTION(mysqli_query, SQL);
POST_HOOK_FUNCTION(mysqli_query, SQL_ERROR);
PRE_HOOK_FUNCTION(mysqli_real_query, SQL);
POST_HOOK_FUNCTION(mysqli_real_query, SQL_ERROR);
PRE_HOOK_FUNCTION(mysqli_prepare, SQL_PREPARED);
POST_HOOK_FUNCTION(mysqli_prepare, SQL_ERROR);

POST_HOOK_FUNCTION_EX(__construct, mysqli, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(__construct, mysqli, SQL_ERROR);
POST_HOOK_FUNCTION_EX(real_connect, mysqli, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(real_connect, mysqli, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(query, mysqli, SQL);
POST_HOOK_FUNCTION_EX(query, mysqli, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(prepare, mysqli, SQL_PREPARED);
POST_HOOK_FUNCTION_EX(prepare, mysqli, SQL_ERROR);

static long fetch_mysqli_errno(const char *function_name, uint32_t param_count, zval params[]);
static std::string fetch_mysqli_error(const char *function_name, uint32_t param_count, zval params[]);

static bool init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p, zend_bool is_real_connect, zend_bool in_ctor)
{
    char *hostname = NULL, *username = NULL, *passwd = NULL, *dbname = NULL, *socket = NULL;
    size_t hostname_len = 0, username_len = 0, passwd_len = 0, dbname_len = 0, socket_len = 0;
    zend_long port = 0, flags = 0;
    zval *object = getThis();
    static char *default_host = INI_STR("mysqli.default_host");
    static char *default_user = INI_STR("mysqli.default_user");
    static char *default_password = INI_STR("mysqli.default_pw");
    static long default_port = INI_INT("mysqli.default_port");
    static char *default_socket = INI_STR("mysqli.default_socket");

    if (default_port <= 0)
    {
        default_port = MYSQL_PORT;
    }

    if (!is_real_connect)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ssssls", &hostname, &hostname_len, &username,
                                  &username_len, &passwd, &passwd_len, &dbname, &dbname_len, &port,
                                  &socket, &socket_len) == FAILURE)
        {
            return false;
        }
    }
    else
    {
        if (in_ctor)
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS(), "|sssslsl", &hostname,
                                      &hostname_len, &username, &username_len, &passwd, &passwd_len,
                                      &dbname, &dbname_len, &port, &socket,
                                      &socket_len, &flags) == FAILURE)
            {
                return false;
            }
        }
        else
        {
            if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "o|sssslsl", &object, &hostname,
                                             &hostname_len, &username, &username_len, &passwd, &passwd_len,
                                             &dbname, &dbname_len, &port, &socket,
                                             &socket_len, &flags) == FAILURE)
            {
                return false;
            }
        }
    }
    if (!username)
    {
        username = default_user;
    }
    if (!passwd)
    {
        passwd = default_password;
    }
    if (!port)
    {
        port = default_port;
    }
    if (!hostname || !hostname_len)
    {
        hostname = default_host;
    }
    if (!socket_len || !socket)
    {
        socket = default_socket;
    }
    sql_connection_p->set_server("mysql");
    sql_connection_p->set_username(SAFE_STRING(username));
    sql_connection_p->set_password(SAFE_STRING(passwd));
    sql_connection_p->set_host(SAFE_STRING(hostname));
    sql_connection_p->set_using_socket(nullptr == hostname || strcmp("localhost", hostname) == 0);
    sql_connection_p->set_socket(SAFE_STRING(socket));
    sql_connection_p->set_port(port);
    return true;
}

static bool init_global_mysqli_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0, 0);
}

static bool init_global_mysqli_real_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1, 0);
}

static bool init_mysqli__construct_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0, 1);
}

static bool init_mysqli_real_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1, 1);
}

static void mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func)
{
    long error_code = fetch_mysqli_errno("mysqli_connect_errno", 0, nullptr);
    if (!mysql_error_code_filtered(error_code))
    {
        return;
    }
    std::string error_msg = fetch_mysqli_error("mysqli_connect_error", 0, nullptr);
    sql_connection_entry conn_entry;
    connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry);
    sql_connect_error_alarm(&conn_entry, std::to_string(error_code), error_msg);
}

//mysqli::__construct
void post_mysqli___construct_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(getThis()) == IS_OBJECT && 0 == fetch_mysqli_errno("mysqli_connect_errno", 0, nullptr) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block();
    }
}

//mysqli::__construct error
void post_mysqli___construct_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(getThis()) == IS_OBJECT)
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry);
    }
}

//mysqli::real_connect

void post_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_TRUE &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block();
    }
}

//mysqli::real_connect error
void post_mysqli_real_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry);
    }
}

//mysqli::query
void pre_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    size_t query_len;
    long resultmode = MYSQLI_STORE_RESULT;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &query, &query_len, &resultmode) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql");
}

void post_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        char *query = NULL;
        size_t query_len;
        long resultmode = MYSQLI_STORE_RESULT;
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &query, &query_len, &resultmode) == FAILURE)
        {
            return;
        }
        long error_code = fetch_mysqli_errno("mysqli_errno", 1, getThis());
        if (!mysql_error_code_filtered(error_code))
        {
            return;
        }
        std::string error_msg = fetch_mysqli_error("mysqli_error", 1, getThis());
        sql_query_error_alarm("mysql", query, std::to_string(error_code), error_msg);
    }
}

//mysqli_connect

void post_global_mysqli_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_OBJECT &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block();
    }
}

//mysqli_connect error
void post_global_mysqli_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry);
    }
}

//mysqli_real_connect
void post_global_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_TRUE &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block();
    }
}

//mysqli_real_connect error
void post_global_mysqli_real_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry);
    }
}

//mysqli_query
void pre_global_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *mysql_link;
    char *query = NULL;
    size_t query_len;
    long resultmode = MYSQLI_STORE_RESULT;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "os|l", &mysql_link, &query, &query_len, &resultmode) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql");
}

void post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        zval *mysql_link;
        char *query = NULL;
        size_t query_len;
        long resultmode = MYSQLI_STORE_RESULT;

        if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "os|l", &mysql_link, &query, &query_len, &resultmode) == FAILURE)
        {
            return;
        }
        long error_code = fetch_mysqli_errno("mysqli_errno", 1, mysql_link);
        if (!mysql_error_code_filtered(error_code))
        {
            return;
        }
        std::string error_msg = fetch_mysqli_error("mysqli_error", 1, mysql_link);
        sql_query_error_alarm("mysql", query, std::to_string(error_code), error_msg);
    }
}

//mysqli_real_query
void pre_global_mysqli_real_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *mysql_link;
    char *query = NULL;
    size_t query_len;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "os", &mysql_link, &query, &query_len) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql");
}

void post_global_mysqli_real_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    size_t query_len;
    zval *mysql_link;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "os", &mysql_link, &query, &query_len) == FAILURE)
    {
        return;
    }

    plugin_sql_check(query, query_len, "mysql");
}

void post_global_mysqli_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    size_t query_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &query, &query_len) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql");
}

void post_mysqli_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static long fetch_mysqli_errno(const char *function_name, uint32_t param_count, zval params[])
{
    long error_code = 0;
    zval function, retval;
    ZVAL_STRING(&function, function_name);
    if (call_user_function(EG(function_table), nullptr, &function, &retval, param_count, params) == SUCCESS &&
        Z_TYPE(retval) == IS_LONG)
    {
        error_code = Z_LVAL(retval);
    }
    zval_ptr_dtor(&function);
    return error_code;
}

static std::string fetch_mysqli_error(const char *function_name, uint32_t param_count, zval params[])
{
    std::string error_msg;
    zval function, retval;
    ZVAL_STRING(&function, function_name);
    if (call_user_function(EG(function_table), nullptr, &function, &retval, param_count, params) == SUCCESS)
    {
        if (Z_TYPE(retval) == IS_STRING)
        {
            error_msg = std::string(Z_STRVAL(retval));
        }
        zval_ptr_dtor(&retval);
    }
    zval_ptr_dtor(&function);
    return error_msg;
}
