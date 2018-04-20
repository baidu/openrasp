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

static void init_mysql_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p, int persistent)
{
    char *user=NULL, *passwd=NULL, *host_and_port=NULL, *socket=NULL, *host=NULL, *tmp=NULL;
    int  user_len = 0, passwd_len = 0, host_len = 0, port = MYSQL_PORT;
    long client_flags = 0; 
    zend_bool new_link = 0;
    static char *default_host = INI_STR("mysql.default_host");
    static char *default_user = INI_STR("mysql.default_user");
    static char *default_password = INI_STR("mysql.default_password");
    static long default_port = INI_INT("mysql.default_port");

	if (default_port <= 0) {
		default_port = MYSQL_PORT;
	}

    if (PG(sql_safe_mode)) 
    {
		if (ZEND_NUM_ARGS()>0) {
			return;
		}
		host_and_port=passwd=NULL;
		user=php_get_current_user(TSRMLS_C);
	} 
    else 
    {
        if (persistent) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!s!s!l", &host_and_port, &host_len,
									&user, &user_len, &passwd, &passwd_len,
									&client_flags)==FAILURE) {
				return;
        	}
		} else {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!s!s!bl", &host_and_port, &host_len,
										&user, &user_len, &passwd, &passwd_len,
										&new_link, &client_flags)==FAILURE) {
				return;
			}
		}
		if (!host_and_port) {
			host_and_port = default_host;
		}
		if (!user) {
			user = default_user;
		}
    }
    if (host_and_port && (tmp=strchr(host_and_port, ':'))) {
		host = estrndup(host_and_port, tmp-host_and_port);
		tmp++;
		if (tmp[0] != '/') {
			port = atoi(tmp);
            sql_connection_p->port = port;
			if ((tmp=strchr(tmp, ':'))) {
				tmp++;
				socket=tmp;
			}
		} else {
			socket = tmp;
		}
        sql_connection_p->host = host;
	} else {
        sql_connection_p->host = estrdup(host_and_port);
        sql_connection_p->port = default_port;
	}
    sql_connection_p->server = "mysql";
    sql_connection_p->username = user ? estrdup(user) : nullptr;
}

static inline void init_mysql_connect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 0);
}

static inline void init_mysql_pconnect_conn_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    init_mysql_connection_entry(INTERNAL_FUNCTION_PARAM_PASSTHRU, sql_connection_p, 1);
}

//mysql_connect
void pre_global_mysql_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_connect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysql_connect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_connect_conn_entry, 0);
    }
}

//mysql_pconnect
void pre_global_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_pconnect_conn_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}
void post_global_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_mysql_pconnect_conn_entry, 0);
    }
}


//mysql_query
void pre_global_mysql_query(INTERNAL_FUNCTION_PARAMETERS)
{    
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "mysql", 1);
}
void post_global_mysql_query(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_check_type_ignored(ZEND_STRL("sqlSlowQuery") TSRMLS_CC)) 
    {
        return;
    } 
    long num_rows = 0;
    if (Z_TYPE_P(return_value) == IS_RESOURCE)
    {
        zval *args[1];
        args[0] = return_value;
        num_rows = fetch_rows_via_user_function("mysql_num_rows", 1, args TSRMLS_CC);
    }
    else if (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value))
    {
        num_rows = fetch_rows_via_user_function("mysql_affected_rows", 0, NULL TSRMLS_CC);
    }
    if (num_rows >= openrasp_ini.slowquery_min_rows)
    {
        slow_query_alarm(num_rows TSRMLS_CC);
    }
}
