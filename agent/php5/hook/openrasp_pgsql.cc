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
#include "utils/regex.h"
#include "hook/data/sql_error_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/sql_object.h"

POST_HOOK_FUNCTION(pg_connect, DB_CONNECTION);
POST_HOOK_FUNCTION(pg_pconnect, DB_CONNECTION);
PRE_HOOK_FUNCTION(pg_query, SQL);
POST_HOOK_FUNCTION(pg_query, SQL_ERROR);
PRE_HOOK_FUNCTION(pg_send_query, SQL);
PRE_HOOK_FUNCTION(pg_prepare, SQL_PREPARED);
POST_HOOK_FUNCTION(pg_prepare, SQL_ERROR);
PRE_HOOK_FUNCTION(pg_send_prepare, SQL_PREPARED);

static bool openrasp_pg_set_error_verbosity(zval *pgsql_link, const std::string &verbose_string, long &old_verbose TSRMLS_DC);
static bool openrasp_pg_set_error_verbosity(zval *pgsql_link, const long &restored_verbose, long &old_verbose TSRMLS_DC);
static bool openrasp_pg_set_error_verbosity(zval *pgsql_link, zval *error_verbose, long &old_verbose TSRMLS_DC);
static void openrasp_detect_pg_last_error(zval *pgsql_link, zval *query TSRMLS_DC);

void parse_connection_string(char *connstring, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    pg_conninfo_parse(connstring,
                      [&sql_connection_obj](const char *pname, const char *pval) {
                          sql_connection_obj.set_name_value(pname, pval);
                      });
}

static bool init_pg_connection_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    char *host = nullptr, *port = nullptr, *options = nullptr, *tty = nullptr, *dbname = nullptr, *connstring = nullptr;
    zval **args[5];
    int i = 0;
    int connect_type = 0;

    if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 5 || zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        return false;
    }
    sql_connection_obj.set_server("pgsql");
    if (ZEND_NUM_ARGS() == 1)
    { /* new style, using connection string */
        connstring = Z_STRVAL_PP(args[0]);
    }
    else if (ZEND_NUM_ARGS() == 2)
    { /* Safe to add conntype_option, since 2 args was illegal */
        connstring = Z_STRVAL_PP(args[0]);
        convert_to_long_ex(args[1]);
        connect_type = Z_LVAL_PP(args[1]);
    }
    if (connstring)
    {
        sql_connection_obj.set_connection_string(connstring);
        parse_connection_string(connstring, sql_connection_obj);
    }
    return true;
}

/**
 * pg_connect
 */
void post_global_pg_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        openrasp::data::SqlConnectionObject sco;
        sql_connection_policy_check(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_pg_connection_entry, sco);
    }
}

/**
 * pg_pconnect 
 */
void post_global_pg_pconnect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    post_global_pg_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/**
 * pg_query
 */
void pre_global_pg_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *pgsql_link = nullptr;
    zval *query = nullptr;
    int argc = ZEND_NUM_ARGS();

    if (argc == 1)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &query) == FAILURE)
        {
            return;
        }
    }
    else
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &pgsql_link, &query) == FAILURE)
        {
            return;
        }
    }

    plugin_sql_check(query, "pgsql" TSRMLS_CC);
    //pg_set_error_verbosity to PGSQL_ERRORS_VERBOSE
    long old_verbose;
    if (openrasp_pg_set_error_verbosity(pgsql_link, "PGSQL_ERRORS_VERBOSE", old_verbose TSRMLS_CC))
    {
        OPENRASP_HOOK_G(origin_pg_error_verbos) = old_verbose;
    }
}

void post_global_pg_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *pgsql_link = nullptr;
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        zval *query = nullptr;
        int argc = ZEND_NUM_ARGS();
        if (argc == 1)
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &query) == FAILURE)
            {
                return;
            }
        }
        else
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &pgsql_link, &query) == FAILURE)
            {
                return;
            }
        }
        openrasp_detect_pg_last_error(pgsql_link, query TSRMLS_CC);
    }
    if (OPENRASP_HOOK_G(origin_pg_error_verbos) != -1)
    {
        //restore error_verbosity
        long useless_verbose;
        openrasp_pg_set_error_verbosity(pgsql_link, OPENRASP_HOOK_G(origin_pg_error_verbos), useless_verbose TSRMLS_CC);
        OPENRASP_HOOK_G(origin_pg_error_verbos) = -1;
    }
}

/**
 * pg_send_query
 */
void pre_global_pg_send_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    pre_global_pg_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_global_pg_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *pgsql_link = nullptr;
    zval *query = nullptr;
    char *stmtname = nullptr;
    int stmtname_len = 0;
    int argc = ZEND_NUM_ARGS();

    if (argc == 2)
    {
        if (zend_parse_parameters(argc TSRMLS_CC, "sz", &stmtname, &stmtname_len, &query) == FAILURE)
        {
            return;
        }
    }
    else
    {
        if (zend_parse_parameters(argc TSRMLS_CC, "rsz",
                                  &pgsql_link, &stmtname, &stmtname_len, &query) == FAILURE)
        {
            return;
        }
    }

    plugin_sql_check(query, "pgsql" TSRMLS_CC);
    //pg_set_error_verbosity to PGSQL_ERRORS_VERBOSE
    long old_verbose;
    if (openrasp_pg_set_error_verbosity(pgsql_link, "PGSQL_ERRORS_VERBOSE", old_verbose TSRMLS_CC))
    {
        OPENRASP_HOOK_G(origin_pg_error_verbos) = old_verbose;
    }
}

void post_global_pg_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *pgsql_link = nullptr;
    if (Z_TYPE_P(return_value) == IS_BOOL && !Z_BVAL_P(return_value))
    {
        zval *query = nullptr;
        char *stmtname = nullptr;
        int stmtname_len = 0;
        int argc = ZEND_NUM_ARGS();

        if (argc == 2)
        {
            if (zend_parse_parameters(argc TSRMLS_CC, "sz", &stmtname, &stmtname_len, &query) == FAILURE)
            {
                return;
            }
        }
        else
        {
            if (zend_parse_parameters(argc TSRMLS_CC, "rsz",
                                      &pgsql_link, &stmtname, &stmtname_len, &query) == FAILURE)
            {
                return;
            }
        }
        openrasp_detect_pg_last_error(pgsql_link, query TSRMLS_CC);
    }
    if (OPENRASP_HOOK_G(origin_pg_error_verbos) != -1)
    {
        //restore error_verbosity
        long useless_verbose;
        openrasp_pg_set_error_verbosity(pgsql_link, OPENRASP_HOOK_G(origin_pg_error_verbos), useless_verbose TSRMLS_CC);
        OPENRASP_HOOK_G(origin_pg_error_verbos) = -1;
    }
}

void pre_global_pg_send_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *pgsql_link = nullptr;
    zval *query = nullptr;
    char *stmtname = nullptr;
    int stmtname_len, id = -1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz", &pgsql_link, &stmtname, &stmtname_len, &query) == FAILURE)
    {
        return;
    }

    if (pgsql_link == NULL && id == -1)
    {
        return;
    }

    plugin_sql_check(query, "pgsql" TSRMLS_CC);
}

bool openrasp_pg_set_error_verbosity(zval *pgsql_link, const std::string &verbose_string, long &old_verbose TSRMLS_DC)
{
    bool result = false;
    zval *error_verbose = nullptr;
    MAKE_STD_ZVAL(error_verbose);
    if (zend_get_constant(verbose_string.c_str(), verbose_string.length(), error_verbose TSRMLS_CC))
    {
        result = openrasp_pg_set_error_verbosity(pgsql_link, error_verbose, old_verbose TSRMLS_CC);
    }
    zval_ptr_dtor(&error_verbose);
    return result;
}

bool openrasp_pg_set_error_verbosity(zval *pgsql_link, const long &restored_verbose, long &old_verbose TSRMLS_DC)
{
    bool result = false;
    zval *z_restored_verbose = nullptr;
    MAKE_STD_ZVAL(z_restored_verbose);
    ZVAL_LONG(z_restored_verbose, restored_verbose);
    result = openrasp_pg_set_error_verbosity(pgsql_link, z_restored_verbose, old_verbose TSRMLS_CC);
    zval_ptr_dtor(&z_restored_verbose);
    return result;
}

bool openrasp_pg_set_error_verbosity(zval *pgsql_link, zval *error_verbose, long &old_verbose TSRMLS_DC)
{
    bool result = false;
    if (nullptr != error_verbose && Z_TYPE_P(error_verbose) == IS_LONG)
    {
        zval *pg_set_error_verbosity_args[2];
        int set_error_verbosity_param_num = 0;
        if (nullptr != pgsql_link)
        {
            pg_set_error_verbosity_args[set_error_verbosity_param_num++] = pgsql_link;
        }
        pg_set_error_verbosity_args[set_error_verbosity_param_num++] = error_verbose;
        zval old_verbosity;
        if (openrasp_call_user_function(EG(function_table), nullptr, "pg_set_error_verbosity", &old_verbosity,
                                        set_error_verbosity_param_num, pg_set_error_verbosity_args TSRMLS_CC))
        {
            if (Z_TYPE(old_verbosity) == IS_LONG && Z_LVAL(old_verbosity) != Z_LVAL_P(error_verbose))
            {
                old_verbose = Z_LVAL(old_verbosity);
                result = true;
            }
            zval_dtor(&old_verbosity);
        }
    }
    return result;
}

void openrasp_detect_pg_last_error(zval *pgsql_link, zval *query TSRMLS_DC)
{
    zval *pg_last_error_args[1];
    int last_error_param_num = 0;
    if (nullptr != pgsql_link)
    {
        pg_last_error_args[0] = pgsql_link;
        last_error_param_num = 1;
    }
    zval last_error;
    if (openrasp_call_user_function(EG(function_table), nullptr, "pg_last_error", &last_error,
                                    last_error_param_num, pg_last_error_args TSRMLS_CC))
    {
        if (Z_TYPE(last_error) == IS_STRING)
        {
            std::string error_msg(Z_STRVAL(last_error), Z_STRLEN(last_error));
            size_t error_found = error_msg.find("ERROR:");
            if (error_found != std::string::npos && error_msg.length() >= error_found + 12)
            {
                std::string error_code = error_msg.substr(error_found + 8, 5);
                if (openrasp::regex_match(error_code.c_str(), "^[0-9A-Z]{5}$"))
                {
                    openrasp::data::SqlErrorObject seo(openrasp::data::SqlObject("pgsql", query), "pgsql", error_code, error_msg);
                    openrasp::checker::V8Detector v8_detector(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
                    v8_detector.run();
                }
            }
        }
        zval_dtor(&last_error);
    }
}