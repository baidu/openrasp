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
POST_HOOK_FUNCTION_EX(mysqli, mysqli, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(mysqli, mysqli, SQL_ERROR);
POST_HOOK_FUNCTION_EX(real_connect, mysqli, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(real_connect, mysqli, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(query, mysqli, SQL);
POST_HOOK_FUNCTION_EX(query, mysqli, SQL_ERROR);
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
PRE_HOOK_FUNCTION_EX(prepare, mysqli, SQL_PREPARED);
POST_HOOK_FUNCTION_EX(prepare, mysqli, SQL_ERROR);

static long fetch_mysqli_errno(const char *function_name, uint32_t param_count, zval *params[] TSRMLS_DC);
static std::string fetch_mysqli_error(const char *function_name, uint32_t param_count, zval *params[] TSRMLS_DC);

static bool init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p, zend_bool is_real_connect, zend_bool in_ctor)
{
    char *hostname = NULL, *username = NULL, *passwd = NULL, *dbname = NULL, *socket = NULL;
    int hostname_len = 0, username_len = 0, passwd_len = 0, dbname_len = 0, socket_len = 0;
    long port = MYSQL_PORT, flags = 0;
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
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssssls", &hostname, &hostname_len, &username, &username_len,
                                  &passwd, &passwd_len, &dbname, &dbname_len, &port, &socket, &socket_len) == FAILURE)
        {
            return false;
        }
    }
    else
    {
        if (in_ctor)
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sssslsl",
                                      &hostname, &hostname_len, &username, &username_len, &passwd, &passwd_len, &dbname,
                                      &dbname_len, &port, &socket, &socket_len, &flags) == FAILURE)
            {
                return false;
            }
        }
        else
        {
            if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "o|sssslsl", &object, &hostname,
                                             &hostname_len, &username, &username_len, &passwd, &passwd_len, &dbname,
                                             &dbname_len, &port, &socket, &socket_len, &flags) == FAILURE)
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
    long error_code = fetch_mysqli_errno("mysqli_connect_errno", 0, nullptr TSRMLS_CC);
    if (!mysql_error_code_filtered(error_code))
    {
        return;
    }
    std::string error_msg = fetch_mysqli_error("mysqli_connect_error", 0, nullptr TSRMLS_CC);
    sql_connection_entry conn_entry;
    connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry);
    sql_connect_error_alarm(&conn_entry, std::to_string(error_code), error_msg TSRMLS_CC);
}

//mysqli::mysqli
void post_mysqli_mysqli_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(this_ptr) == IS_OBJECT && 0 == fetch_mysqli_errno("mysqli_connect_errno", 0, nullptr TSRMLS_CC) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
    }
}

//mysqli::mysqli error
void post_mysqli_mysqli_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry);
    }
}

//mysqli::real_connect
void post_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
    }
}

//mysqli::real_connect error
void post_mysqli_real_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry);
    }
}

//mysqli::query
void pre_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    int query_len;
    long resultmode = MYSQLI_STORE_RESULT;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &query, &query_len, &resultmode) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        char *query = NULL;
        int query_len;
        long resultmode = MYSQLI_STORE_RESULT;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &query, &query_len, &resultmode) == FAILURE)
        {
            return;
        }
        zval *args[1];
        args[0] = this_ptr;
        int param_num = 1;
        long error_code = fetch_mysqli_errno("mysqli_errno", param_num, args TSRMLS_CC);
        if (!mysql_error_code_filtered(error_code))
        {
            return;
        }
        std::string error_msg = fetch_mysqli_error("mysqli_error", param_num, args TSRMLS_CC);
        sql_query_error_alarm("mysql", query, std::to_string(error_code), error_msg TSRMLS_CC);
    }
}

//mysqli_connect
void post_global_mysqli_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_OBJECT && 0 == fetch_mysqli_errno("mysqli_connect_errno", 0, nullptr TSRMLS_CC) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
    }
}

//mysqli_connect error
void post_global_mysqli_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry);
    }
}

//mysqli_real_connect
void post_global_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
    }
}

//mysqli_real_connect error
void post_global_mysqli_real_connect_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        mysqli_connect_error_intercept(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry);
    }
}

//mysqli_query
void pre_global_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *mysql_link;
    char *query = NULL;
    int query_len;
    long resultmode = MYSQLI_STORE_RESULT;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os|l", &mysql_link, &query, &query_len, &resultmode) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        zval *mysql_link;
        char *query = NULL;
        int query_len;
        long resultmode = MYSQLI_STORE_RESULT;

        if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os|l", &mysql_link, &query, &query_len, &resultmode) == FAILURE)
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
        long error_code = fetch_mysqli_errno("mysqli_errno", param_num, args TSRMLS_CC);
        if (!mysql_error_code_filtered(error_code))
        {
            return;
        }
        std::string error_msg = fetch_mysqli_error("mysqli_error", param_num, args TSRMLS_CC);
        sql_query_error_alarm("mysql", query, std::to_string(error_code), error_msg TSRMLS_CC);
    }
}

//mysqli_real_query
void pre_global_mysqli_real_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *mysql_link;
    char *query = NULL;
    int query_len;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os", &mysql_link, &query, &query_len) == FAILURE)
    {
        return;
    }

    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_global_mysqli_real_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    int query_len;
    zval *mysql_link;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os", &mysql_link, &query, &query_len) == FAILURE)
    {
        return;
    }

    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_global_mysqli_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *query = NULL;
    int query_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &query, &query_len) == FAILURE)
    {
        return;
    }
    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_mysqli_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_mysqli_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static long fetch_mysqli_errno(const char *function_name, uint32_t param_count, zval *params[] TSRMLS_DC)
{
    long error_code = 0;
    zval function, retval;
    INIT_ZVAL(function);
    ZVAL_STRING(&function, function_name, 0);
    if (call_user_function(EG(function_table), nullptr, &function, &retval, param_count, params TSRMLS_CC) == SUCCESS &&
        Z_TYPE(retval) == IS_LONG)
    {
        error_code = Z_LVAL(retval);
    }
    return error_code;
}

static std::string fetch_mysqli_error(const char *function_name, uint32_t param_count, zval *params[] TSRMLS_DC)
{
    std::string error_msg;
    zval function, retval;
    INIT_ZVAL(function);
    ZVAL_STRING(&function, function_name, 0);
    if (call_user_function(EG(function_table), nullptr, &function, &retval, param_count, params TSRMLS_CC) == SUCCESS)
    {
        if (Z_TYPE(retval) == IS_STRING)
        {
            error_msg = std::string(Z_STRVAL(retval));
        }
        zval_dtor(&retval);
    }
    return error_msg;
}
