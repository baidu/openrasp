#ifndef OPENRASP_SQL_H
#define OPENRASP_SQL_H

#include "openrasp_ini.h"
#include "openrasp_hook.h"

#define MYSQLI_STORE_RESULT 0
#define MYSQLI_USE_RESULT 	1
#define MYSQL_PORT          3306

typedef struct sql_connection_entry_t {
	char *server = nullptr;
    char *host = nullptr;
    char *username = nullptr;
} sql_connection_entry;

typedef void (*init_connection_t)(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p);

void slow_query_alarm(int rows TSRMLS_DC);
zend_bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func, int enforce_policy);
void check_query_clause(INTERNAL_FUNCTION_PARAMETERS, char *server, int num);
long fetch_rows_via_user_function(const char *f_name_str, zend_uint param_count, zval *params[] TSRMLS_DC);

//************************ mysql ************************
//mysql_connect
void pre_global_mysql_connect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysql_connect(INTERNAL_FUNCTION_PARAMETERS);

//mysql_pconnect
void pre_global_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS);

//mysql_query
void pre_global_mysql_query(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysql_query(INTERNAL_FUNCTION_PARAMETERS);

//************************ mysqli ************************
//mysqli::mysqli 
void pre_mysqli_mysqli(INTERNAL_FUNCTION_PARAMETERS);
void post_mysqli_mysqli(INTERNAL_FUNCTION_PARAMETERS);

//mysqli::real_connect 
void pre_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS);
void post_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS);

//mysqli::query 
void pre_mysqli_query(INTERNAL_FUNCTION_PARAMETERS);
void post_mysqli_query(INTERNAL_FUNCTION_PARAMETERS);

//mysqli_connect
void pre_global_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS);

//mysqli_real_connect
void pre_global_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS);

//mysqli_query
void pre_global_mysqli_query(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysqli_query(INTERNAL_FUNCTION_PARAMETERS);

//mysqli_real_query
void pre_global_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS);
void post_global_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS);

//************************ pgsql ************************
//pg_connect
void pre_global_pg_connect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_pg_connect(INTERNAL_FUNCTION_PARAMETERS);

//pg_pconnect
void pre_global_pg_pconnect(INTERNAL_FUNCTION_PARAMETERS);
void post_global_pg_pconnect(INTERNAL_FUNCTION_PARAMETERS);

//pg_query
void pre_global_pg_query(INTERNAL_FUNCTION_PARAMETERS);
void post_global_pg_query(INTERNAL_FUNCTION_PARAMETERS);

//pg_send_query
void pre_global_pg_send_query(INTERNAL_FUNCTION_PARAMETERS);
void post_global_pg_send_query(INTERNAL_FUNCTION_PARAMETERS);

//pg_get_result
void pre_global_pg_get_result(INTERNAL_FUNCTION_PARAMETERS);
void post_global_pg_get_result(INTERNAL_FUNCTION_PARAMETERS);

//************************ pdo ************************
void pre_pdo_query(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo_query(INTERNAL_FUNCTION_PARAMETERS);
void pre_pdo_exec(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo_exec(INTERNAL_FUNCTION_PARAMETERS);
void pre_pdo___construct(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo___construct(INTERNAL_FUNCTION_PARAMETERS);

//************************ sqlite3 ************************
//SQLite3::exec
void pre_sqlite3_exec(INTERNAL_FUNCTION_PARAMETERS);
void post_sqlite3_exec(INTERNAL_FUNCTION_PARAMETERS);

//SQLite3::query
void pre_sqlite3_query(INTERNAL_FUNCTION_PARAMETERS);
void post_sqlite3_query(INTERNAL_FUNCTION_PARAMETERS);

//SQLite3::querySingle
void pre_sqlite3_querySingle(INTERNAL_FUNCTION_PARAMETERS);
void post_sqlite3_querySingle(INTERNAL_FUNCTION_PARAMETERS);

#endif //OPENRASP_SQL_H