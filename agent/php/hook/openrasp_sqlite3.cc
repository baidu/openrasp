#include "openrasp_sql.h"

//sqlite3::exec
void pre_sqlite3_exec_ex(INTERNAL_FUNCTION_PARAMETERS, char *server)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, server, 1);
}
void post_sqlite3_exec_ex(INTERNAL_FUNCTION_PARAMETERS, char *server){}

//sqlite3::query
void pre_sqlite3_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, server, 1);
}
void post_sqlite3_query_ex(INTERNAL_FUNCTION_PARAMETERS, char *server){}

//sqlite3::querySingle
void pre_sqlite3_querySingle_ex(INTERNAL_FUNCTION_PARAMETERS, char *server)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, server, 1);
}
void post_sqlite3_querySingle_ex(INTERNAL_FUNCTION_PARAMETERS, char *server){}