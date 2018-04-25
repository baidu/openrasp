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
		if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "o|sssslsl", &object,
										&hostname, &hostname_len, &username, &username_len, &passwd, &passwd_len, &dbname, &dbname_len, &port, &socket, &socket_len,
										&flags) == FAILURE) {
			return;
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

static void init_mysqli_connect_connection(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysqli_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0, 0);
}

static void init_mysqli_real_connect_connection(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
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
void pre_mysqli_mysqli(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_mysqli_mysqli(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli__construct_conn_entry, 0);
    }
}


//mysqli::real_connect 
void pre_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_conn_entry, 0);
    }
}


//mysqli::query
void pre_mysqli_query(INTERNAL_FUNCTION_PARAMETERS)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "mysql", 1);
}
void post_mysqli_query(INTERNAL_FUNCTION_PARAMETERS) 
{
    if (openrasp_check_type_ignored(ZEND_STRL("sqlSlowQuery") TSRMLS_CC)) 
    {
        return;
    }        
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
void pre_global_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_connect_connection, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_connect_connection, 0);
    }
}


//mysqli_real_connect
void pre_global_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_connection, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value))
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysqli_real_connect_connection, 0);
    }
}


//mysqli_query
void pre_global_mysqli_query(INTERNAL_FUNCTION_PARAMETERS)
{    
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "mysql", 2);
}
void post_global_mysqli_query(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_check_type_ignored(ZEND_STRL("sqlSlowQuery") TSRMLS_CC)) 
    {
        return;
    }        
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
void pre_global_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS)
{    
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "mysql", 2);
}
void post_global_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS){}



