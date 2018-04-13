#include "openrasp_hook.h"

//sqlite3::exec
void pre_sqlite3_exec(INTERNAL_FUNCTION_PARAMETERS)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "sqlite", 1);
}
void post_sqlite3_exec(INTERNAL_FUNCTION_PARAMETERS){}

//sqlite3::query
void pre_sqlite3_query(INTERNAL_FUNCTION_PARAMETERS)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "sqlite", 1);
}
void post_sqlite3_query(INTERNAL_FUNCTION_PARAMETERS){}

//sqlite3::querySingle
void pre_sqlite3_querySingle(INTERNAL_FUNCTION_PARAMETERS)
{
    check_query_clause(INTERNAL_FUNCTION_PARAM_PASSTHRU, "sqlite", 1);
}
void post_sqlite3_querySingle(INTERNAL_FUNCTION_PARAMETERS){}