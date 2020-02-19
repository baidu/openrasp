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

#ifndef OPENRASP_SQL_H
#define OPENRASP_SQL_H

#include "hook/data/sql_connection_object.h"
#include "openrasp_v8.h"
#include "openrasp_utils.h"

typedef bool (*init_sql_connection_t)(INTERNAL_FUNCTION_PARAMETERS, openrasp::data::SqlConnectionObject &sql_connection_obj);

void plugin_sql_check(zval *query, const std::string &server);
void sql_connection_policy_check(INTERNAL_FUNCTION_PARAMETERS, init_sql_connection_t connection_init_func, openrasp::data::SqlConnectionObject &sql_connection_obj);
void pg_conninfo_parse(char *connstring, std::function<void(const char *pname, const char *pval)> info_store_func);

#endif