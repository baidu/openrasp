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

//SQLite3::exec
void pre_sqlite3_exec_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_sqlite3_exec_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//SQLite3::query
void pre_sqlite3_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_sqlite3_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//SQLite3::querySingle
void pre_sqlite3_querySingle_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_sqlite3_querySingle_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli::mysqli 
void pre_mysqli_mysqli_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_mysqli_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli::real_connect 
void pre_mysqli_real_connect_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_real_connect_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli::query 
void pre_mysqli_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysql_connect
void pre_mysql_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysql_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysql_pconnect
void pre_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysql_pconnect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysql_query
void pre_mysql_query(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysql_query(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli_connect
void pre_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli_real_connect
void pre_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_real_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli_query
void pre_mysqli_query(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_query(INTERNAL_FUNCTION_PARAMETERS, char *server);

//mysqli_real_query
void pre_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_mysqli_real_query(INTERNAL_FUNCTION_PARAMETERS, char *server);

//pg_connect
void pre_pg_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_pg_connect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//pg_pconnect
void pre_pg_pconnect(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_pg_pconnect(INTERNAL_FUNCTION_PARAMETERS, char *server);

//pg_query
void pre_pg_query(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_pg_query(INTERNAL_FUNCTION_PARAMETERS, char *server);

//pg_send_query
void pre_pg_send_query(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_pg_send_query(INTERNAL_FUNCTION_PARAMETERS, char *server);

//pg_get_result
void pre_pg_get_result(INTERNAL_FUNCTION_PARAMETERS, char *server);
void post_pg_get_result(INTERNAL_FUNCTION_PARAMETERS, char *server);

void pre_pdo_query_ex(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo_query_ex(INTERNAL_FUNCTION_PARAMETERS);
void pre_pdo_exec_ex(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo_exec_ex(INTERNAL_FUNCTION_PARAMETERS);
void pre_pdo___construct_ex(INTERNAL_FUNCTION_PARAMETERS);
void post_pdo___construct_ex(INTERNAL_FUNCTION_PARAMETERS);

#endif //OPENRASP_SQL_H