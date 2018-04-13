#include "openrasp_hook.h"

extern "C" {
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
}

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
    sql_connection_p->username = estrdup(username);
    sql_connection_p->host = estrdup(data_source);
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
        if (!stmt->dbh) {	
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