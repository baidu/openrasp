/*
 * Copyright 2017-2019 Baidu Inc.
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

#include "openrasp_sql.h"
#include "openrasp_hook.h"

PRE_HOOK_FUNCTION_EX(exec, sqlite3, SQL);
PRE_HOOK_FUNCTION_EX(query, sqlite3, SQL);
PRE_HOOK_FUNCTION_EX(querysingle, sqlite3, SQL);

//sqlite3::exec
void pre_sqlite3_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	char *sql;
	size_t sql_len;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s", &sql, &sql_len))
	{
		return;
	}
	plugin_sql_check(sql, sql_len, "sqlite");
}

//sqlite3::query
void pre_sqlite3_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	pre_sqlite3_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

//sqlite3::querySingle
void pre_sqlite3_querysingle_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	char *sql;
	size_t sql_len;
	zend_bool entire_row = 0;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|b", &sql, &sql_len, &entire_row))
	{
		return;
	}
	plugin_sql_check(sql, sql_len, "sqlite");
}