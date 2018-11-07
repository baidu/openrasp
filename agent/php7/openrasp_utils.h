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

#ifndef OPENRASP_UTILS_H
#define OPENRASP_UTILS_H

#include "openrasp.h"
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <functional>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "ext/standard/php_string.h"

#ifdef __cplusplus
}
#endif

const char *fetch_url_scheme(const char *filename);

int recursive_mkdir(const char *path, int len, int mode);
bool write_str_to_file(const char *file, std::ios_base::openmode mode, const char *content, size_t content_len);
bool get_entire_file_content(const char *file, std::string &content);
void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins, std::function<bool(const char *filename)> file_filter, bool use_abs_path = false);

std::vector<std::string> format_debug_backtrace_arr();
void format_debug_backtrace_arr(zval *backtrace_arr);
std::string format_debug_backtrace_str();
void format_debug_backtrace_str(zval *backtrace_str);
std::string json_encode_from_zval(zval *value);

void fetch_if_addrs(std::map<std::string, std::string> &if_addr_map);
void fetch_hw_addrs(std::vector<std::string> &hw_addrs);
bool fetch_source_in_ip_packets(char *local_ip, size_t len, char *url);

char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey);
bool fetch_outmost_long_from_ht(HashTable *ht, const char *arKey, long *result);
HashTable *fetch_outmost_hashtable_from_ht(HashTable *ht, const char *arKey);

bool start_with(const std::string &str, const std::string &prefix);
bool end_with(const std::string &str, const std::string &suffix);

#endif