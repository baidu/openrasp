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

#include "openrasp.h"
#include "openrasp_utils.h"

class SqlConnectionEntry
{
public:
  typedef enum connection_policy_type_t
  {
    USER,
    PASSWORD
  } connection_policy_type;

private:
  std::string connection_string;
  std::string server;
  std::string host;
  std::string username;
  std::string socket;
  std::string password;
  int port = 0;
  bool using_socket = true;
  std::string get_type_name(SqlConnectionEntry::connection_policy_type type);
  long get_type_id(SqlConnectionEntry::connection_policy_type type);

public:
  void set_connection_string(std::string connection_string);
  std::string get_connection_string() const;

  void set_server(std::string server);
  std::string get_server() const;

  void set_host(std::string host);
  std::string get_host() const;

  void set_username(std::string username);
  std::string get_username() const;

  void set_password(std::string password);
  std::string get_password() const;

  void set_socket(std::string socket);
  std::string get_socket() const;

  void set_port(int port);
  int get_port() const;

  std::string build_policy_msg(connection_policy_type type);
  ulong build_hash_code(connection_policy_type type);

  void set_using_socket(bool using_socket);
  bool get_using_socket() const;

  void set_name_value(const char *name, const char *val);

  void connection_entry_policy_log(connection_policy_type type);
  bool check_high_privileged();
  bool check_weak_password();
};

typedef SqlConnectionEntry sql_connection_entry;

typedef bool (*init_connection_t)(INTERNAL_FUNCTION_PARAMETERS, sql_connection_entry *sql_connection_p);

void plugin_sql_check(char *query, int query_len, char *server TSRMLS_DC);
bool check_database_connection_username(INTERNAL_FUNCTION_PARAMETERS, init_connection_t connection_init_func,
                                        int enforce_policy);
bool mysql_error_code_filtered(long err_code);
void sql_query_error_alarm(char *server, char *query, const std::string &err_code, const std::string &err_msg TSRMLS_DC);
void sql_connect_error_alarm(sql_connection_entry *sql_connection_p, const std::string &err_code, const std::string &err_msg TSRMLS_DC);

void pg_conninfo_parse(char *connstring, std::function<void(const char *pname, const char *pval)> info_store_func);

#endif