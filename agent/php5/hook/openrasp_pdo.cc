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
#include "ext/pdo/php_pdo_driver.h"
#include "ext/standard/php_var.h"
#include "zend_ini.h"
}

POST_HOOK_FUNCTION_EX(__construct, pdo, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(__construct, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(query, pdo, SQL);
POST_HOOK_FUNCTION_EX(query, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(exec, pdo, SQL);
POST_HOOK_FUNCTION_EX(exec, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(prepare, pdo, SQL_PREPARED);
POST_HOOK_FUNCTION_EX(prepare, pdo, SQL_ERROR);

static void fetch_pdo_error_info(const char *driver_name, zval *statement, std::string &error_code, std::string &errro_msg TSRMLS_DC);
static void fetch_pdo_exception_info(const char *driver_name, zval *object, std::string &error_code, std::string &errro_msg TSRMLS_DC);

extern void parse_connection_string(char *connstring, sql_connection_entry *sql_connection_p);

static char *dsn_from_uri(char *uri, char *buf, size_t buflen TSRMLS_DC)
{
    php_stream *stream;
    char *dsn = NULL;

    stream = php_stream_open_wrapper(uri, "rb", REPORT_ERRORS, NULL);
    if (stream)
    {
        dsn = php_stream_get_line(stream, buf, buflen, NULL);
        php_stream_close(stream);
    }
    return dsn;
}

static bool init_pdo_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    char *data_source;
    int data_source_len;
    char *colon;
    char *username = NULL, *password = NULL;
    int usernamelen, passwordlen;
    zval *options = NULL;
    char alt_dsn[512];

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s!s!a!", &data_source, &data_source_len,
                                         &username, &usernamelen, &password, &passwordlen, &options))
    {
        return false;
    }
    sql_connection_p->set_connection_string(data_source);
    /* parse the data source name */
    colon = strchr(data_source, ':');

    if (!colon)
    {
        /* let's see if this string has a matching dsn in the php.ini */
        char *ini_dsn = NULL;

        snprintf(alt_dsn, sizeof(alt_dsn), "pdo.dsn.%s", data_source);
        if (FAILURE == cfg_get_string(alt_dsn, &ini_dsn))
        {
            return false;
        }

        data_source = ini_dsn;
        colon = strchr(data_source, ':');

        if (!colon)
        {
            return false;
        }
    }

    if (!strncmp(data_source, "uri:", sizeof("uri:") - 1))
    {
        /* the specified URI holds connection details */
        data_source = dsn_from_uri(data_source + sizeof("uri:") - 1, alt_dsn, sizeof(alt_dsn) TSRMLS_CC);
        if (!data_source)
        {
            return false;
        }
        colon = strchr(data_source, ':');
        if (!colon)
        {
            return false;
        }
    }
    static const char *server_names[] = {"mysql", "mssql", "oci", "pgsql"};
    int server_size = sizeof(server_names) / sizeof(server_names[0]);
    for (int index = 0; index < server_size; ++index)
    {
        if (strncmp(server_names[index], data_source, strlen(server_names[index])) == 0)
        {
            sql_connection_p->set_server((const char *)server_names[index]);
        }
    }
    if (sql_connection_p->get_server() == "mysql")
    {
        struct pdo_data_src_parser mysql_vars[] = {
            {"charset", NULL, 0},
            {"dbname", "", 0},
            {"host", "localhost", 0},
            {"port", "3306", 0},
            {"unix_socket", NULL, 0},
        };
        php_pdo_parse_data_source(colon + 1, strlen(colon + 1), mysql_vars, 5);
        sql_connection_p->set_host(mysql_vars[2].optval);
        sql_connection_p->set_using_socket(nullptr == mysql_vars[2].optval || strcmp("localhost", mysql_vars[2].optval) == 0);
        sql_connection_p->set_port(atoi(mysql_vars[3].optval));
        sql_connection_p->set_socket(SAFE_STRING(mysql_vars[4].optval));
        if (username)
        {
            sql_connection_p->set_username(username);
        }
        if (password)
        {
            sql_connection_p->set_password(password);
        }
        for (int i = 0; i < 5; i++)
        {
            if (mysql_vars[i].freeme)
            {
                efree(mysql_vars[i].optval);
            }
        }
    }
    else if (sql_connection_p->get_server() == "pgsql")
    {
        char *e, *p, *conn_str = nullptr;
        char *dhn_data_source = estrdup(colon + 1);
        e = (char *)dhn_data_source + strlen(dhn_data_source);
        p = (char *)dhn_data_source;
        while ((p = (char *)memchr(p, ';', (e - p))))
        {
            *p = ' ';
        }
        if (username && password)
        {
            spprintf(&conn_str, 0, "%s user=%s password=%s", dhn_data_source, username, password);
        }
        else if (username)
        {
            spprintf(&conn_str, 0, "%s user=%s", dhn_data_source, username);
        }
        else if (password)
        {
            spprintf(&conn_str, 0, "%s password=%s", dhn_data_source, password);
        }
        else
        {
            spprintf(&conn_str, 0, "%s", (char *)dhn_data_source);
        }
        parse_connection_string(conn_str, sql_connection_p);
        efree(conn_str);
        efree(dhn_data_source);
    }
    else
    {
        //It is not supported at present
    }
    return true;
}

void pre_pdo_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = reinterpret_cast<pdo_dbh_t *>(zend_object_store_get_object(getThis() TSRMLS_CC));
    char *statement;
    int statement_len;

    if (!ZEND_NUM_ARGS() ||
        FAILURE == zend_parse_parameters(1 TSRMLS_CC, "s", &statement, &statement_len))
    {
        return;
    }
    plugin_sql_check(statement, statement_len, const_cast<char *>(dbh->driver->driver_name) TSRMLS_CC);
}

void post_pdo_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = reinterpret_cast<pdo_dbh_t *>(zend_object_store_get_object(getThis() TSRMLS_CC));
    char *driver_name = (char *)dbh->driver->driver_name;
    if (strcmp(driver_name, "mysql"))
    {
        return;
    }
    char *statement;
    int statement_len;

    if (!ZEND_NUM_ARGS() ||
        FAILURE == zend_parse_parameters(1 TSRMLS_CC, "s", &statement, &statement_len))
    {
        return;
    }
    std::string error_code;
    std::string error_msg;

    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        if (dbh->error_mode == PDO_ERRMODE_EXCEPTION)
        {
            if (EG(exception) &&
                Z_TYPE_P(EG(exception)) == IS_OBJECT &&
                instanceof_function(Z_OBJCE_P(EG(exception)), php_pdo_get_exception() TSRMLS_CC))
            {
                fetch_pdo_exception_info(driver_name, EG(exception), error_code, error_msg TSRMLS_CC);
            }
        }
        else
        {
            fetch_pdo_error_info(driver_name, this_ptr, error_code, error_msg TSRMLS_CC);
        }
    }
    if (!error_code.empty())
    {
        sql_query_error_alarm(driver_name, statement, error_code, error_msg TSRMLS_CC);
    }
}

void pre_pdo_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_pdo_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void post_pdo_exec_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_pdo_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void post_pdo___construct_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(this_ptr) == IS_OBJECT && !EG(exception) &&
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_pdo_connection_entry,
                                           OPENRASP_CONFIG(security.enforce_policy) ? 1 : 0))
    {
        handle_block(TSRMLS_C);
    }
}

void post_pdo___construct_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (EG(exception) &&
        Z_TYPE_P(EG(exception)) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(EG(exception)), php_pdo_get_exception() TSRMLS_CC))
    {
        sql_connection_entry conn_entry;
        if (!init_pdo_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry))
        {
            return;
        }
        std::string error_code;
        std::string error_msg;
        fetch_pdo_exception_info(conn_entry.get_server().c_str(), EG(exception), error_code, error_msg TSRMLS_CC);
        if (!error_code.empty())
        {
            sql_connect_error_alarm(&conn_entry, error_code, error_msg TSRMLS_CC);
        }
    }
}

void pre_pdo_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = reinterpret_cast<pdo_dbh_t *>(zend_object_store_get_object(getThis() TSRMLS_CC));
    char *statement;
    int statement_len;
    zval *options = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &statement,
                                         &statement_len, &options))
    {
        return;
    }
    plugin_sql_check(statement, statement_len, const_cast<char *>(dbh->driver->driver_name) TSRMLS_CC);
}

void post_pdo_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_pdo_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void fetch_pdo_error_info(const char *driver_name, zval *statement, std::string &error_code, std::string &errro_msg TSRMLS_DC)
{
    zval function_name, retval;
    INIT_ZVAL(function_name);
    ZVAL_STRING(&function_name, "errorinfo", 0);
    if (call_user_function(EG(function_table), &statement, &function_name, &retval, 0, nullptr TSRMLS_CC) == SUCCESS)
    {
        if (Z_TYPE(retval) == IS_ARRAY)
        {
            zval **tmp;
            if (zend_hash_index_find(Z_ARRVAL(retval), 2, (void **)&tmp) == SUCCESS &&
                Z_TYPE_PP(tmp) == IS_STRING)
            {
                errro_msg = std::string(Z_STRVAL_PP(tmp));
            }
            if (zend_hash_index_find(Z_ARRVAL(retval), 1, (void **)&tmp) == SUCCESS)
            {
                if (0 == strcmp(driver_name, "mysql") &&
                    Z_TYPE_PP(tmp) == IS_LONG &&
                    mysql_error_code_filtered(Z_LVAL_PP(tmp)))
                {
                    error_code = std::to_string(Z_LVAL_PP(tmp));
                }
            }
        }
        zval_dtor(&retval);
    }
}

static void fetch_pdo_exception_info(const char *driver_name, zval *object, std::string &error_code, std::string &errro_msg TSRMLS_DC)
{
    zval *code = zend_read_property(php_pdo_get_exception(), object, "code", sizeof("code") - 1, 1 TSRMLS_CC);
    if (Z_TYPE_P(code) == IS_LONG)
    {
        error_code = std::to_string(Z_LVAL_P(code));
    }
    else if (Z_TYPE_P(code) == IS_STRING)
    {
        error_code = std::string(Z_STRVAL_P(code));
    }
    zval *message = zend_read_property(php_pdo_get_exception(), object, "message", sizeof("message") - 1, 1 TSRMLS_CC);
    if (Z_TYPE_P(message) == IS_STRING)
    {
        errro_msg = std::string(Z_STRVAL_P(message));
    }
}