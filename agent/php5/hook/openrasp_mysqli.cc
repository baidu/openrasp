/*
 * Copyright 2017-2018 Baidu Inc.
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

extern "C" {
#include "zend_ini.h"
}
/**
 * mysqli相关hook点
 */
HOOK_FUNCTION_EX(mysqli, mysqli, DB_CONNECTION);
HOOK_FUNCTION_EX(real_connect, mysqli, DB_CONNECTION);
PRE_HOOK_FUNCTION_EX(query, mysqli, SQL);
POST_HOOK_FUNCTION_EX(query, mysqli, SQL_SLOW_QUERY);
HOOK_FUNCTION(mysqli_connect, DB_CONNECTION);
HOOK_FUNCTION(mysqli_real_connect, DB_CONNECTION);
PRE_HOOK_FUNCTION(mysqli_query, SQL);
POST_HOOK_FUNCTION(mysqli_query, SQL_SLOW_QUERY);
PRE_HOOK_FUNCTION(mysqli_real_query, SQL);
PRE_HOOK_FUNCTION(mysqli_prepare, SQL_PREPARED);
PRE_HOOK_FUNCTION_EX(prepare, mysqli, SQL_PREPARED);

static void init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p, zend_bool is_real_connect, zend_bool in_ctor)
{
    char *hostname = NULL, *username=NULL, *passwd=NULL, *dbname=NULL, *socket=NULL;
    int	  hostname_len = 0, username_len = 0, passwd_len = 0, dbname_len = 0, socket_len = 0;
    long  port = MYSQL_PORT, flags = 0;
    zval *object = getThis();
    static char *default_host = INI_STR("mysqli.default_host");
    static char *default_user = INI_STR("mysqli.default_user");
    static char *default_password = INI_STR("mysqli.default_pw");
    static long default_port = INI_INT("mysqli.default_port");
    static char *default_socket = INI_STR("mysqli.default_socket");

	if (default_port <= 0) {
		default_port = MYSQL_PORT;
	}

    if (!is_real_connect) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssssls", &hostname, &hostname_len, &username, &username_len,
									&passwd, &passwd_len, &dbname, &dbname_len, &port, &socket, &socket_len) == FAILURE) {
			return;
		}
	} else {
        if (in_ctor)
        {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sssslsl",
                                            &hostname, &hostname_len, &username, &username_len, &passwd, &passwd_len, &dbname, &dbname_len, &port, &socket, &socket_len,
                                            &flags) == FAILURE) {
                return;
            }
        }
        else
        {
            if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "o|sssslsl", &object,
                                            &hostname, &hostname_len, &username, &username_len, &passwd, &passwd_len, &dbname, &dbname_len, &port, &socket, &socket_len,
                                            &flags) == FAILURE) {
                return;
            }
        }


	}
    if (!username){
		username = default_user;
	}
	if (!port){
		port = default_port;
	}
	if (!hostname || !hostname_len) {
		hostname = default_host;
	}
    if (!socket_len || !socket) {
		socket = default_socket;
	} 
    sql_connection_p->server = "mysql";
    sql_connection_p->username = estrdup(username);

    if (hostname && strcmp(hostname, "localhost") != 0) {
        sql_connection_p->host = estrdup(hostname);
        sql_connection_p->port = port;
    }
}

static void init_global_mysqli_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0, 0);
}

static void init_global_mysqli_real_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1, 0);
}

static void init_mysqli__construct_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0, 1);
}

static void init_mysqli_real_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1, 1);
}

//mysqli::mysqli
void pre_mysqli_mysqli_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_mysqli_mysqli_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry, 0);
    }
}


//mysqli::real_connect 
void pre_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry, 0);
    }
}


//mysqli::query
void pre_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	char				*query = NULL;
	int 				query_len;
	long 				resultmode = MYSQLI_STORE_RESULT;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &query, &query_len, &resultmode) == FAILURE) {
		return;
	}
    sql_type_handler(query, query_len, "mysql" TSRMLS_CC);
}

void post_mysqli_query_SQL_SLOW_QUERY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS) 
{       
    long resultmode = MYSQLI_STORE_RESULT;
    int argc = ZEND_NUM_ARGS();
    if (2 == argc)
    {
        zval ***ppp_args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
        if(zend_get_parameters_array_ex(argc, ppp_args) == SUCCESS &&
        Z_TYPE_PP(ppp_args[1]) == IS_LONG)
        {        
            resultmode =  Z_LVAL_PP(ppp_args[1]);            
        }
        efree(ppp_args);         
    } 
    long num_rows = 0;
    if (resultmode == MYSQLI_STORE_RESULT && Z_TYPE_P(return_value) == IS_OBJECT)
    {             
        zval *args[1];
        args[0] = return_value;
        num_rows = fetch_rows_via_user_function("mysqli_num_rows", 1, args TSRMLS_CC);
    }
    else if (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value))
    {
        zval *args[1];
        args[0] = this_ptr;
        num_rows = fetch_rows_via_user_function("mysqli_affected_rows", 1, args TSRMLS_CC);
    }
    if (num_rows >= openrasp_ini.slowquery_min_rows)
    {
        slow_query_alarm(num_rows TSRMLS_CC);
    }
}


//mysqli_connect
void pre_global_mysqli_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysqli_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_connect_conn_entry, 0);
    }
}


//mysqli_real_connect
void pre_global_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysqli_real_connect_DB_CONNECTION(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value))
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_global_mysqli_real_connect_conn_entry, 0);
    }
}


//mysqli_query
void pre_global_mysqli_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval				*mysql_link;
	char				*query = NULL;
	int 				query_len;
	long 				resultmode = MYSQLI_STORE_RESULT;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os|l", &mysql_link, &query, &query_len, &resultmode) == FAILURE) {
		return;
	}
    sql_type_handler(query, query_len, "mysql" TSRMLS_CC);
}
void post_global_mysqli_query_SQL_SLOW_QUERY(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{      
    long resultmode = MYSQLI_STORE_RESULT;
    int argc = ZEND_NUM_ARGS();    
    zval ***ppp_args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
    if(zend_get_parameters_array_ex(argc, ppp_args) == SUCCESS)
    {
        if (3 == argc && Z_TYPE_PP(ppp_args[2]) == IS_LONG)
        {
            resultmode =  Z_LVAL_PP(ppp_args[2]);            
        }        
    }         
    long num_rows = 0;
    if (resultmode == MYSQLI_STORE_RESULT && Z_TYPE_P(return_value) == IS_OBJECT)
    {             
        zval *args[1];
        args[0] = return_value;
        num_rows = fetch_rows_via_user_function("mysqli_num_rows", 1, args TSRMLS_CC);
    }
    else if (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value))
    {
        zval *args[1];
        args[0] = *ppp_args[0];
        num_rows = fetch_rows_via_user_function("mysqli_affected_rows", 1, args TSRMLS_CC);
    }
    efree(ppp_args);
    if (num_rows >= openrasp_ini.slowquery_min_rows)
    {
        slow_query_alarm(num_rows TSRMLS_CC);
    }
}


//mysqli_real_query
void pre_global_mysqli_real_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval		*mysql_link;
	char		*query = NULL;
	int			query_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os", &mysql_link, &query, &query_len) == FAILURE) {
		return;
	}

    sql_type_handler(query, query_len, "mysql" TSRMLS_CC);
}

void pre_global_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{    
	char			*query = NULL;
	int				query_len;
	zval			*mysql_link;
	
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "os",&mysql_link, &query, &query_len) == FAILURE) {
		return;
	}

    sql_type_handler(query, query_len, "mysql" TSRMLS_CC);
}

void pre_mysqli_prepare_SQL_PREPARED(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	char			*query = NULL;
	int				query_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &query, &query_len) == FAILURE) {
		return;
	}
    sql_type_handler(query, query_len, "mysql" TSRMLS_CC);
}


