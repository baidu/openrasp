/*
 * Copyright 2017-2021 Baidu Inc.
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

#include "hook/data/sql_error_object.h"
#include "hook/checker/v8_detector.h"
#include "hook/data/sql_object.h"
#include "openrasp_sql.h"
#include "openrasp_hook.h"

PRE_HOOK_FUNCTION_EX(exec, sqlite3, SQL);
POST_HOOK_FUNCTION_EX(exec, sqlite3, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(query, sqlite3, SQL);
POST_HOOK_FUNCTION_EX(query, sqlite3, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(prepare, sqlite3, SQL);
POST_HOOK_FUNCTION_EX(prepare, sqlite3, SQL_ERROR);
PRE_HOOK_FUNCTION_EX(querysingle, sqlite3, SQL);
POST_HOOK_FUNCTION_EX(querysingle, sqlite3, SQL_ERROR);

static void sqlite_query_error_intercept(zval *query, zval *object_p);

//sqlite3::exec
void pre_sqlite3_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *sql = nullptr;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &sql))
	{
		return;
	}
	plugin_sql_check(sql, "sqlite");
}

//sqlite3::query
void pre_sqlite3_query_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	pre_sqlite3_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void pre_sqlite3_prepare_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	pre_sqlite3_exec_SQL(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

//sqlite3::querySingle
void pre_sqlite3_querysingle_SQL(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	zval *sql = nullptr;
	zend_bool entire_row = 0;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &sql, &entire_row))
	{
		return;
	}
	plugin_sql_check(sql, "sqlite");
}

void post_sqlite3_exec_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	if (Z_TYPE_P(return_value) == IS_FALSE)
	{
		zval *sql = nullptr;

		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &sql))
		{
			return;
		}
		sqlite_query_error_intercept(sql, getThis());
	}
}

void post_sqlite3_query_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	post_sqlite3_exec_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void post_sqlite3_prepare_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	post_sqlite3_exec_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
void post_sqlite3_querysingle_SQL_ERROR(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
	if (Z_TYPE_P(return_value) == IS_FALSE)
	{
		zval *sql = nullptr;
		zend_bool entire_row = 0;

		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &sql, &entire_row))
		{
			return;
		}
		sqlite_query_error_intercept(sql, getThis());
	}
}

void sqlite_query_error_intercept(zval *query, zval *object_p)
{
	int error_code = 0;
	zval z_error_code;
	if (openrasp_call_user_function(EG(function_table), object_p, "lasterrorcode", &z_error_code, 0, nullptr))
	{
		if (Z_TYPE(z_error_code) == IS_LONG)
		{
			error_code = Z_LVAL(z_error_code);
		}
		zval_dtor(&z_error_code);
	}
	std::string error_msg;
	zval z_error_msg;
	if (openrasp_call_user_function(EG(function_table), object_p, "lasterrormsg", &z_error_msg, 0, nullptr))
	{
		if (Z_TYPE(z_error_msg) == IS_STRING)
		{
			error_msg = std::string(Z_STRVAL(z_error_msg), Z_STRLEN(z_error_msg));
		}
		zval_dtor(&z_error_msg);
	}
	openrasp::data::SqlErrorObject seo(openrasp::data::SqlObject("sqlite", query), "sqlite", error_code, error_msg);
	openrasp::checker::V8Detector v8_detector(seo, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
	v8_detector.run();
}