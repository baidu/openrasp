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

#include "utils/regex.h"
#include "hook/data/sql_error_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/sql_object.h"
#include "openrasp_sql.h"
#include "openrasp_hook.h"

extern "C"
{
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
#include "Zend/zend_objects_API.h"
}

POST_HOOK_FUNCTION_EX(__construct, pdo, DB_CONNECTION);
POST_HOOK_FUNCTION_EX(__construct, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(query, pdo, SQL);
POST_HOOK_FUNCTION_EX(query, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(exec, pdo, SQL);
POST_HOOK_FUNCTION_EX(exec, pdo, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(prepare, pdo, SQL_PREPARED);
POST_HOOK_FUNCTION_EX(prepare, pdo, SQL_ERROR);

const static std::set<std::string> supported_server = {"mysql", "pgsql", "sqlite"};

static void error_info_check(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *error_info);
static void pdo_exception_intercept(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *object);
static void pdo_error_info_intercept(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *statement);

extern void parse_connection_string(char *connstring, openrasp::data::SqlConnectionObject &sql_connection_obj);

static bool is_server_supported(const std::string &server)
{
    auto found = supported_server.find(server);
    return found != supported_server.end();
}

static char *dsn_from_uri(char *uri, char *buf, size_t buflen)
{
    php_stream *stream = nullptr;
    char *dsn = nullptr;

    stream = php_stream_open_wrapper(uri, "rb", REPORT_ERRORS, nullptr);
    if (stream)
    {
        dsn = php_stream_get_line(stream, buf, buflen, nullptr);
        php_stream_close(stream);
    }
    return dsn;
}

static bool init_pdo_connection_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    char *data_source = nullptr;
    size_t data_source_len = 0;
    char *colon = nullptr;
    char *username = nullptr, *password = nullptr;
    size_t usernamelen = 0;
    size_t passwordlen = 0;
    zval *options = nullptr;
    char alt_dsn[512];

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|s!s!a!", &data_source, &data_source_len,
                                         &username, &usernamelen, &password, &passwordlen, &options))
    {
        return false;
    }

    sql_connection_obj.set_connection_string(data_source);
    /* parse the data source name */
    colon = strchr(data_source, ':');

    if (!colon)
    {
        /* let's see if this string has a matching dsn in the php.ini */
        char *ini_dsn = nullptr;

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
        data_source = dsn_from_uri(data_source + sizeof("uri:") - 1, alt_dsn, sizeof(alt_dsn));
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
            sql_connection_obj.set_server((const char *)server_names[index]);
        }
    }
    if (sql_connection_obj.get_server() == "mysql")
    {
        struct pdo_data_src_parser mysql_vars[] = {
            {"charset", nullptr, 0},
            {"dbname", "", 0},
            {"host", "localhost", 0},
            {"port", "3306", 0},
            {"unix_socket", nullptr, 0},
        };
        int matches = php_pdo_parse_data_source(colon + 1, strlen(colon + 1), mysql_vars, 5);
        sql_connection_obj.set_host(mysql_vars[2].optval);
        sql_connection_obj.set_using_socket(nullptr == mysql_vars[2].optval || strcmp("localhost", mysql_vars[2].optval) == 0);
        sql_connection_obj.set_port(atoi(mysql_vars[3].optval));
        sql_connection_obj.set_socket(SAFE_STRING(mysql_vars[4].optval));
        if (username)
        {
            sql_connection_obj.set_username(username);
        }
        if (password)
        {
            sql_connection_obj.set_password(password);
        }
        for (int i = 0; i < 5; i++)
        {
            if (mysql_vars[i].freeme)
            {
                efree(mysql_vars[i].optval);
            }
        }
    }
    else if (sql_connection_obj.get_server() == "pgsql")
    {
        char *e = nullptr;
        char *p = nullptr;
        char *conn_str = nullptr;
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
        parse_connection_string(conn_str, sql_connection_obj);
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
    pdo_dbh_t *dbh = Z_PDO_DBH_P(getThis());
    zval *statement = nullptr;

    if (!ZEND_NUM_ARGS() ||
        FAILURE == zend_parse_parameters(1, "z", &statement))
    {
        return;
    }

    plugin_sql_check(statement, dbh->driver->driver_name);
}

void post_pdo_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = Z_PDO_DBH_P(getThis());
    std::string driver_name = std::string(dbh->driver->driver_name);
    if (!is_server_supported(driver_name))
    {
        return;
    }
    zval *statement = nullptr;
    if (!ZEND_NUM_ARGS() ||
        FAILURE == zend_parse_parameters(1, "z", &statement))
    {
        return;
    }
    openrasp::data::SqlObject sql_obj(driver_name, statement);
    if (Z_TYPE_P(return_value) == IS_FALSE)
    {
        if (dbh->error_mode == PDO_ERRMODE_EXCEPTION)
        {
            if (EG(exception) && EG(exception)->ce == php_pdo_get_exception())
            {
                zval object;
                ZVAL_OBJ(&object, EG(exception));
                pdo_exception_intercept(sql_obj, driver_name, &object);
            }
        }
        else
        {
            pdo_error_info_intercept(sql_obj, driver_name, getThis());
        }
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
    if (Z_TYPE_P(getThis()) == IS_OBJECT && !EG(exception))
    {
        openrasp::data::SqlConnectionObject sco;
        sql_connection_policy_check(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_pdo_connection_entry, sco);
    }
}

void post_pdo___construct_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (EG(exception) && EG(exception)->ce == php_pdo_get_exception())
    {
        openrasp::data::SqlConnectionObject sco;
        init_pdo_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sco);
        if (is_server_supported(sco.get_server()))
        {
            zval object;
            ZVAL_OBJ(&object, EG(exception));
            pdo_exception_intercept(sco, sco.get_server().c_str(), &object);
        }
    }
}

void pre_pdo_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = Z_PDO_DBH_P(getThis());
    zval *statement = nullptr;
    zval *options = nullptr;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|a", &statement, &options))
    {
        return;
    }
    plugin_sql_check(statement, dbh->driver->driver_name);
}

void post_pdo_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_pdo_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void error_info_check(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *error_info)
{
    if (nullptr != error_info && Z_TYPE_P(error_info) == IS_ARRAY)
    {
        zval *tmp = nullptr;
        std::string error_msg;
        if ((tmp = zend_hash_index_find(Z_ARRVAL_P(error_info), 2)) != nullptr &&
            Z_TYPE_P(tmp) == IS_STRING)
        {
            error_msg = std::string(Z_STRVAL_P(tmp));
        }
        ulong code_index = driver_name == "pgsql" ? 0 : 1;
        if ((tmp = zend_hash_index_find(Z_ARRVAL_P(error_info), code_index)) != nullptr)
        {
            if (Z_TYPE_P(tmp) == IS_LONG)
            {
                openrasp::data::SqlErrorObject seo(v8_material, driver_name, Z_LVAL_P(tmp), error_msg);
                openrasp::checker::V8Detector error_checker(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                error_checker.run();
            }
            else if (Z_TYPE_P(tmp) == IS_STRING)
            {
                openrasp::data::SqlErrorObject seo(v8_material, driver_name, Z_STRVAL_P(tmp), error_msg);
                openrasp::checker::V8Detector error_checker(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                error_checker.run();
            }
        }
    }
}

void pdo_error_info_intercept(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *statement)
{
    zval retval;
    if (openrasp_call_user_function(EG(function_table), statement, "errorinfo", &retval, 0, nullptr))
    {
        error_info_check(v8_material, driver_name, &retval);
        zval_ptr_dtor(&retval);
    }
}

static void pdo_exception_intercept(const openrasp::data::V8Material &v8_material, const std::string &driver_name, zval *object)
{
    zval rv;
    zval *error_info = zend_read_property(php_pdo_get_exception(), object, "errorInfo", sizeof("errorInfo") - 1, 1, &rv);
    if (nullptr != error_info && Z_TYPE_P(error_info) == IS_ARRAY)
    {
        error_info_check(v8_material, driver_name, error_info);
    }
    else
    {
        if (driver_name == "mysql" ||
            driver_name == "sqlite")
        {
            zval *code = zend_read_property(php_pdo_get_exception(), object, "code", sizeof("code") - 1, 1, &rv);
            if (nullptr == code)
            {
                return;
            }
            std::string error_msg;
            zval *message = zend_read_property(php_pdo_get_exception(), object, "message", sizeof("message") - 1, 1, &rv);
            if (Z_TYPE_P(message) == IS_STRING)
            {
                error_msg = std::string(Z_STRVAL_P(message));
            }
            if (Z_TYPE_P(code) == IS_LONG)
            {
                openrasp::data::SqlErrorObject seo(v8_material, driver_name, Z_LVAL_P(code), error_msg);
                openrasp::checker::V8Detector error_checker(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                error_checker.run();
            }
            else if (Z_TYPE_P(code) == IS_STRING)
            {
                openrasp::data::SqlErrorObject seo(v8_material, driver_name, Z_STRVAL_P(code), error_msg);
                openrasp::checker::V8Detector error_checker(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                error_checker.run();
            }
        }
        else if (driver_name == "pgsql")
        {
            zval *message = zend_read_property(php_pdo_get_exception(), object, "message", sizeof("message") - 1, 1, &rv);
            if (nullptr != message && Z_TYPE_P(message) == IS_STRING)
            {
                std::string error_msg = std::string(Z_STRVAL_P(message), Z_STRLEN_P(message));
                if (openrasp::regex_search(error_msg.c_str(), "^SQLSTATE\\[[0-9A-Z]{5}\\] .*"))
                {
                    std::string error_code = error_msg.substr(9, 5);
                    openrasp::data::SqlErrorObject seo(v8_material, driver_name, error_code, error_msg);
                    openrasp::checker::V8Detector v8_detector(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                    v8_detector.run();
                }
            }
        }
    }
}