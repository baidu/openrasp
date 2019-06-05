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
#include <string>

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

static long fetch_mysql_errno(uint32_t param_count, zval *params[] TSRMLS_DC);
static std::string fetch_mysql_error(uint32_t param_count, zval *params[] TSRMLS_DC);

static bool init_mysql_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p, int persistent)
{
    char *user = NULL, *passwd = NULL, *host_and_port = NULL, *socket = NULL, *host = NULL, *tmp = NULL;
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
        host_and_port = passwd = NULL;
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
    sql_connection_p->set_server("mysql");
    socket = default_socket;
    bool using_socket = false;

    if (host_and_port && (tmp = strchr(host_and_port, ':')))
    {
        host = estrndup(host_and_port, tmp - host_and_port);
        tmp++;
        if (tmp[0] != '/')
        {
            port = atoi(tmp);
            sql_connection_p->set_port(port);
            if ((tmp = strchr(tmp, ':')))
            {
                tmp++;
                socket = tmp;
                using_socket = true;
            }
        }
        else
        {
            socket = tmp;
            using_socket = true;
        }
        sql_connection_p->set_host(host);
        using_socket = (strcmp("localhost", host) == 0);
    }
    else
    {
        sql_connection_p->set_host(SAFE_STRING(host_and_port));
        using_socket = (host_and_port == nullptr || strncmp(host_and_port, "localhost", strlen("localhost")) == 0);
        sql_connection_p->set_port(default_port);
    }
    sql_connection_p->set_using_socket(using_socket);
    sql_connection_p->set_socket(SAFE_STRING(socket));
    sql_connection_p->set_username(SAFE_STRING(user));
    sql_connection_p->set_password(SAFE_STRING(passwd));
    return true;
}

static inline bool init_mysql_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0);
}

static inline bool init_mysql_pconnect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    return init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1);
}

static void mysql_connect_error_intercept(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func)
{
    long error_code = fetch_mysql_errno(0, nullptr TSRMLS_CC);
    if (!mysql_error_code_filtered(error_code))
    {
        return;
    }
    std::string error_msg = fetch_mysql_error(0, nullptr TSRMLS_CC);
    sql_connection_entry conn_entry;
    connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry);
    sql_connect_error_alarm(&conn_entry, std::to_string(error_code), error_msg TSRMLS_CC);
}

//mysql_connect
void post_global_mysql_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_RESOURCE &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_connect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
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
    if (Z_TYPE_P(return_value) == IS_RESOURCE &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_pconnect_conn_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
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
    char *query;
    int query_len;
    zval *mysql_link = NULL;

    if (UNLIKELY(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &query, &query_len, &mysql_link) == FAILURE))
    {
        return;
    }

    plugin_sql_check(query, query_len, "mysql" TSRMLS_CC);
}

void post_global_mysql_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        char *query;
        int query_len;
        zval *mysql_link = nullptr;
        if (UNLIKELY(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &query, &query_len, &mysql_link) == FAILURE))
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
        if (!mysql_error_code_filtered(error_code))
        {
            return;
        }
        std::string error_msg = fetch_mysql_error(param_num, args TSRMLS_CC);
        sql_query_error_alarm("mysql", query, std::to_string(error_code), error_msg TSRMLS_CC);
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
