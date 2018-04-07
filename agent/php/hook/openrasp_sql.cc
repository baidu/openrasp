#include "openrasp_sql.h"
#include "openrasp_ini.h"
#include <string>
#include <map>

extern "C" {
#include "ext/pdo/php_pdo_driver.h"
#include "zend_ini.h"
#include "openrasp_shared_alloc.h"
}

/**
 * sql connection alarm
 */
static void connection_via_default_username_policy(char *check_message TSRMLS_DC)
{           
    zval *params_result = nullptr;
    MAKE_STD_ZVAL(params_result);
    array_init(params_result);
    add_assoc_string(params_result, "message", check_message, 1);
    add_assoc_long(params_result, "policy_id", 3006);
    policy_info(params_result TSRMLS_CC);
    zval_ptr_dtor(&params_result);
}

void slow_query_alarm(int rows TSRMLS_DC)
{
    zval *attack_params = nullptr;
    MAKE_STD_ZVAL(attack_params);
    ZVAL_LONG(attack_params, rows);
    zval *plugin_message = nullptr;
    MAKE_STD_ZVAL(plugin_message);
    char *message_str = nullptr;
    spprintf(&message_str, 0, _("slow query: read %d rows via sql statement - exceed %d"), rows, openrasp_ini.slowquery_min_rows);
    ZVAL_STRING(plugin_message, message_str, 1);
    efree(message_str);
    openrasp_buildin_php_risk_handle(0, "sqlSlowQuery", 100, attack_params, plugin_message TSRMLS_CC);
}

zend_bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func, int enforce_policy)
{
    static const std::multimap<std::string, std::string> database_username_blacklists = {
        {"mysql", "root"},
        {"mssql", "sa"},
        {"pgsql", "postgres"},
        {"oci", "dbsnmp"},
        {"oci", "sysman"},
        {"oci", "system"},
        {"oci", "sys"}
    };
    sql_connection_entry conn_entry;
    char *check_message = nullptr;
    zend_bool need_block= 0;
    connection_init_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_entry);
    if (conn_entry.server && conn_entry.username && conn_entry.host)
    {
        auto pos = database_username_blacklists.equal_range(std::string(conn_entry.server));
        while (pos.first != pos.second)
        {
            if (std::string(conn_entry.username) == pos.first->second) 
            {
                spprintf(&check_message, 0, _("connect %s server via the default username:%s"), conn_entry.server, conn_entry.username);
                break;
            }
            pos.first++;
        }
        if (check_message)
        {
            if (enforce_policy)
            {
                connection_via_default_username_policy(check_message TSRMLS_CC);
                need_block = 1;
            }
            else
            { 
                ulong connection_hash = zend_inline_hash_func(conn_entry.host, strlen(conn_entry.host));
                openrasp_shared_alloc_lock(TSRMLS_C);
                if (!openrasp_shared_hash_exist(connection_hash, OPENRASP_LOG_G(formatted_date_suffix)))
                {
                    connection_via_default_username_policy(check_message TSRMLS_CC);
                }
                openrasp_shared_alloc_unlock(TSRMLS_C);
            }
            efree(check_message);
        }
    }
    if (conn_entry.host)
    {
        efree(conn_entry.host);
    }
    if (conn_entry.username)
    {
        efree(conn_entry.username);
    }
    return need_block;
}

/*  check sql query clause
*/
void check_query_clause(INTERNAL_FUNCTION_PARAMETERS, char *server, int num)
{    
    if (openrasp_check_type_ignored(ZEND_STRL("sql") TSRMLS_CC)) 
    {
        return;
    }
    int argc = MIN(num, ZEND_NUM_ARGS());
    zval ***args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
    if (argc == num &&
        zend_get_parameters_array_ex(argc, args) == SUCCESS &&
        Z_TYPE_PP(args[num - 1]) == IS_STRING)
    {        
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        add_assoc_zval(params, "query", *args[num - 1]);        
        Z_ADDREF_P(*args[num - 1]);
        add_assoc_string(params, "server", server, 1);
        check("sql", params TSRMLS_CC);
    }
    efree(args);
}

long fetch_rows_via_user_function(const char *f_name_str, zend_uint param_count, zval *params[] TSRMLS_DC)
{
    zval function_name, retval;
    INIT_ZVAL(function_name);
    ZVAL_STRING(&function_name, f_name_str, 0);
    if (call_user_function(EG(function_table), nullptr, &function_name, &retval, param_count, params TSRMLS_CC) == SUCCESS
    && Z_TYPE(retval) == IS_LONG)
    {
        return Z_LVAL(retval);
    }
    return 0;
}


