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

#ifndef OPENRASP_UTILS_H
#define OPENRASP_UTILS_H

#include "openrasp.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

const char *fetch_url_scheme(const char *filename);

int recursive_mkdir(const char *path, int len, int mode TSRMLS_DC);
bool get_entire_file_content(const char *file, std::string &content);
void openrasp_scandir(const std::string dir_abs, std::vector<std::string> &plugins,
                      std::function<bool(const char *filename)> file_filter, bool use_abs_path = false);

std::vector<std::string> format_source_code_arr(TSRMLS_D);
void format_source_code_arr(zval *source_code_arr TSRMLS_DC);
std::vector<std::string> format_debug_backtrace_arr(TSRMLS_D);
std::string format_debug_backtrace_str(TSRMLS_D);
void format_debug_backtrace_str(zval *backtrace_str TSRMLS_DC);

char *fetch_outmost_string_from_ht(HashTable *ht, const char *arKey);
std::string json_encode_from_zval(zval *value TSRMLS_DC);

char *fetch_request_body(size_t max_len TSRMLS_DC);
bool need_alloc_shm_current_sapi();
std::string convert_to_header_key(char *key, size_t length);
bool openrasp_parse_url(const std::string &origin_url, std::string &scheme, std::string &host, std::string &port);

#endif