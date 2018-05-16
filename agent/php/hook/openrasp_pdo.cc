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
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
}

extern void parse_connection_string(char *connstring, sql_connection_entry *sql_connection_p);

static char *dsn_from_uri(char *uri, char *buf, size_t buflen TSRMLS_DC)
{
	php_stream *stream;
	char *dsn = NULL;

	stream = php_stream_open_wrapper(uri, "rb", REPORT_ERRORS, NULL);
	if (stream) {
		dsn = php_stream_get_line(stream, buf, buflen, NULL);
		php_stream_close(stream);
	}
	return dsn;
}

static void init_pdo_connection_entry(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p)
{
    char *data_source;
    int data_source_len;
    char *colon;
    char *username=NULL, *password=NULL;
    int usernamelen, passwordlen;
    zval *options = NULL;
    char alt_dsn[512];

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s!s!a!", &data_source, &data_source_len,
                &username, &usernamelen, &password, &passwordlen, &options)) {
        return;
    }

    /* parse the data source name */
    colon = strchr(data_source, ':');

    if (!colon) {
        /* let's see if this string has a matching dsn in the php.ini */
        char *ini_dsn = NULL;

        snprintf(alt_dsn, sizeof(alt_dsn), "pdo.dsn.%s", data_source);
        if (FAILURE == cfg_get_string(alt_dsn, &ini_dsn)) {
            return;
        }

        data_source = ini_dsn;
        colon = strchr(data_source, ':');

        if (!colon) {
            return;
        }
    }

    if (!strncmp(data_source, "uri:", sizeof("uri:")-1)) {
        /* the specified URI holds connection details */
        data_source = dsn_from_uri(data_source + sizeof("uri:")-1, alt_dsn, sizeof(alt_dsn) TSRMLS_CC);
        if (!data_source) {
            return;
        }
        colon = strchr(data_source, ':');
        if (!colon) {
            return;
        }
    }
    static const char *server_names[] = {"mysql","mssql","oci","pgsql"};
    int server_size = sizeof(server_names)/sizeof(server_names[0]);
    for (int index = 0; index < server_size; ++index)
    {
        if (strncmp(server_names[index], data_source, strlen(server_names[index])) == 0)
        {
            sql_connection_p->server = (char*)server_names[index];
        }
    }
    if (strcmp(sql_connection_p->server, "mysql") == 0)
    {
        struct pdo_data_src_parser mysql_vars[] = {
            { "charset",  NULL,	0 },
            { "dbname",   "",	0 },
            { "host",   "localhost",	0 },
            { "port",   "3306",	0 },
            { "unix_socket",  NULL,	0 },
	    };
        php_pdo_parse_data_source(data_source, data_source_len, mysql_vars, 5);
        sql_connection_p->host = estrdup(mysql_vars[2].optval);
        sql_connection_p->port = atoi(mysql_vars[3].optval);
        sql_connection_p->username = estrdup(username);
    }
    else if (strcmp(sql_connection_p->server, "pgsql") == 0)
    {
        char *e, *p, *conn_str = nullptr;
        e = (char *) data_source + strlen(data_source);
        p = (char *) data_source;
        while ((p = (char *)memchr(p, ';', (e - p)))) {
            *p = ' ';
        }
        if (username && password) {
            spprintf(&conn_str, 0, "%s user=%s password=%s", data_source, username, password);
        } else if (username) {
            spprintf(&conn_str, 0, "%s user=%s", data_source, username);
        } else if (password) {
            spprintf(&conn_str, 0, "%s password=%s", data_source, password);
        } else {
            spprintf(&conn_str, 0, "%s", (char *) data_source);
        }
        parse_connection_string(conn_str, sql_connection_p);
    }
    else
    {
        //It is not supported at present
    }
}

static void pdo_pre_process(INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_dbh_t *dbh = reinterpret_cast<pdo_dbh_t*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char*>(dbh->driver->driver_name), 1);
}

void pre_pdo_query(INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_pre_process(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void post_pdo_query(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_check_type_ignored(ZEND_STRL("sqlSlowQuery") TSRMLS_CC)) 
    {
        return;
    }    
    if (Z_TYPE_P(return_value) == IS_OBJECT)
    {
        pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object(return_value TSRMLS_CC);
        if (!stmt || !stmt->dbh) {	
            return;	
        }	
        if (stmt->row_count >= openrasp_ini.slowquery_min_rows)
        {
            slow_query_alarm(stmt->row_count TSRMLS_CC);      
        }
    }    
}

void pre_pdo_exec(INTERNAL_FUNCTION_PARAMETERS)
{
    pdo_pre_process(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void post_pdo_exec(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_check_type_ignored(ZEND_STRL("sqlSlowQuery") TSRMLS_CC)) 
    {
        return;
    }    
    if (Z_TYPE_P(return_value) == IS_LONG)
    {	
        if (Z_LVAL_P(return_value) >= openrasp_ini.slowquery_min_rows)
        {
            slow_query_alarm(Z_LVAL_P(return_value) TSRMLS_CC);
        }
    } 
}

void pre_pdo___construct(INTERNAL_FUNCTION_PARAMETERS)
{
    if (openrasp_ini.enforce_policy)
    {        
        if (check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_pdo_connection_entry, 1))
        {
            handle_block(TSRMLS_C);
        }
    }
}

void post_pdo___construct(INTERNAL_FUNCTION_PARAMETERS)
{
    if (!openrasp_ini.enforce_policy && Z_TYPE_P(this_ptr) == IS_OBJECT)
    {
        check_database_connection_username(INTERNAL_FUNCTION_PARAM_PASSTHRU, init_pdo_connection_entry, 0);
    }
}