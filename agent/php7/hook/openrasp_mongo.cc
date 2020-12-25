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

#include "openrasp_hook.h"
#include "openrasp_v8.h"
#include "openrasp_sql.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/mongo_object.h"
#include "hook/checker/policy_detector.h"
#include "hook/data/mongo_connection_object.h"
#include "hook/data/sql_password_object.h"

/**
 * mongo相关hook点
 */
PRE_HOOK_FUNCTION_EX(delete, mongodb_0_driver_0_bulkwrite, MONGO);
PRE_HOOK_FUNCTION_EX(update, mongodb_0_driver_0_bulkwrite, MONGO);
PRE_HOOK_FUNCTION_EX(__construct, mongodb_0_driver_0_query, MONGO);
PRE_HOOK_FUNCTION_EX(__construct, mongodb_0_driver_0_command, MONGO);

POST_HOOK_FUNCTION_EX(__construct, mongodb_0_driver_0_manager, DB_CONNECTION);

static void handle_mongo_uri_string(char *uri_string, size_t uri_string_len, openrasp::data::SqlConnectionObject &sql_connection_obj,
                                    std::string const &default_uri = "mongodb://127.0.0.1/");
static void mongo_connection_policy_check(INTERNAL_FUNCTION_PARAMETERS, init_sql_connection_t connection_init_func,
                                          openrasp::data::SqlConnectionObject &sql_connection_obj);

void handle_mongo_uri_string(char *uri_string, size_t uri_string_len, openrasp::data::SqlConnectionObject &sql_connection_obj,
                             std::string const &default_uri)
{
    std::string uri_tb_be_parsed = default_uri;
    if (uri_string_len)
    {
        uri_tb_be_parsed = std::string(uri_string, uri_string_len);
    }
    sql_connection_obj.parse(uri_tb_be_parsed);
}

static void handle_mongo_options(HashTable *ht, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    std::string password = fetch_outmost_string_from_ht(ht, "password");
    if (!password.empty())
    {
        sql_connection_obj.set_password(password);
    }
    std::string username = fetch_outmost_string_from_ht(ht, "username");
    if (!username.empty())
    {
        sql_connection_obj.set_username(username);
    }
}

static bool init_mongodb_connection_entry(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    char *uri_string = NULL;
    size_t uri_string_len = 0;
    zval *options = NULL;
    zval *driverOptions = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s!a/!a/!", &uri_string, &uri_string_len, &options, &driverOptions) == FAILURE)
    {
        return false;
    }

    handle_mongo_uri_string(uri_string, uri_string_len, sql_connection_obj);
    if (options && Z_TYPE_P(options) == IS_ARRAY)
    {
        handle_mongo_options(Z_ARRVAL_P(options), sql_connection_obj);
    }
    sql_connection_obj.set_server("mongodb");
    return true;
}

static void mongo_plugin_check(const std::string &query_str, const std::string &classname, const std::string &method)
{
    openrasp::data::MongoObject mongo_obj("mongodb", query_str, classname, method);
    openrasp::checker::V8Detector v8_detector(mongo_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
    v8_detector.run();
}

//for mongodb extension
static void mongodb_plugin_check(zval *query, const std::string &classname, const std::string &method)
{
    std::string query_json;
    if (Z_TYPE_P(query) == IS_ARRAY)
    {
        query_json = json_encode_from_zval(query);
    }
    else if (Z_TYPE_P(query) == IS_OBJECT)
    {
        zval fromphp, serialized_bson;
        ZVAL_STRING(&fromphp, "mongodb\\bson\\fromphp");
        if (call_user_function(EG(function_table), nullptr, &fromphp, &serialized_bson, 1, query) == SUCCESS)
        {
            if (Z_TYPE(serialized_bson) == IS_STRING)
            {
                zval tojson, serialized_json;
                ZVAL_STRING(&tojson, "mongodb\\bson\\tojson");
                if (call_user_function(EG(function_table), nullptr, &tojson, &serialized_json, 1, &serialized_bson) == SUCCESS)
                {
                    if (Z_TYPE(serialized_json) == IS_STRING)
                    {
                        query_json = std::string(Z_STRVAL(serialized_json));
                    }
                    zval_ptr_dtor(&serialized_json);
                }
                zval_ptr_dtor(&tojson);
            }
            zval_ptr_dtor(&serialized_bson);
        }
        zval_ptr_dtor(&fromphp);
    }
    mongo_plugin_check(query_json, classname, method);
}

void pre_mongodb_0_driver_0_bulkwrite_delete_MONGO(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *zquery, *zoptions = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "A|a!", &zquery, &zoptions) == FAILURE)
    {
        return;
    }
    mongodb_plugin_check(zquery, "MongoDB\\Driver\\Bulkwrite", "delete");
}

void pre_mongodb_0_driver_0_bulkwrite_update_MONGO(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *zquery, *zupdate, *zoptions = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "AA|a!", &zquery, &zupdate, &zoptions) == FAILURE)
    {
        return;
    }

    mongodb_plugin_check(zquery, "MongoDB\\Driver\\Bulkwrite", "update");
}

void pre_mongodb_0_driver_0_query___construct_MONGO(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *filter;
    zval *options = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "A|a!", &filter, &options) == FAILURE)
    {
        return;
    }
    mongodb_plugin_check(filter, "MongoDB\\Driver\\Query", "__construct");
}

void pre_mongodb_0_driver_0_command___construct_MONGO(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *document;
    zval *options = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "A|a!", &document, &options) == FAILURE)
    {
        return;
    }

    mongodb_plugin_check(document, "MongoDB\\Driver\\Command", "__construct");
}

void post_mongodb_0_driver_0_manager___construct_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (Z_TYPE_P(getThis()) == IS_OBJECT && !EG(exception))
    {
        openrasp::data::MongoConnectionObject mongo_connection_obj;
        mongo_connection_policy_check(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mongodb_connection_entry, mongo_connection_obj);
    }
}

void mongo_connection_policy_check(INTERNAL_FUNCTION_PARAMETERS, init_sql_connection_t connection_init_func, openrasp::data::SqlConnectionObject &sql_connection_obj)
{
    if (connection_init_func)
    {
        if (connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_obj))
        {
            openrasp::data::SqlPasswordObject spo(sql_connection_obj);
            openrasp::checker::PolicyDetector weak_passwd_detector(spo);
            weak_passwd_detector.run();
        }
    }
}