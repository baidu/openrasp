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

#ifndef OPENRASP_V8_H
#define OPENRASP_V8_H

#include "openrasp.h"
#include "hook/checker/check_result.h"
#include "php/header.h"

namespace openrasp
{
class openrasp_v8_process_globals
{
public:
  Snapshot *snapshot_blob = nullptr;
  std::mutex mtx;
  std::string plugin_config = "global.checkPoints=['command','directory','fileUpload','readFile','request','requestEnd','sql','sql_exception','writeFile','xxe','ognl','deserialization','reflection','webdav','ssrf','include','eval','copy','rename','loadLibrary','ssrfRedirect','deleteFile','mongodb','response'];";
  std::vector<PluginFile> plugin_src_list;
  std::once_flag init_v8_once;
};
extern openrasp_v8_process_globals process_globals;
CheckResult Check(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout = 100);
v8::Local<v8::Value> NewV8ValueFromZval(v8::Isolate *isolate, zval *val);
v8::Local<v8::ObjectTemplate> CreateRequestContextTemplate(Isolate *isolate);
void extract_buildin_action(Isolate *isolate, std::map<std::string, std::string> &buildin_action_map);
std::vector<int64_t> extract_int64_array(Isolate *isolate, const std::string &value, int limit, const std::vector<int64_t> &default_value = std::vector<int64_t>());
std::vector<std::string> extract_string_array(Isolate *isolate, const std::string &value, int limit, const std::vector<std::string> &default_value = std::vector<std::string>());
int64_t extract_int64(Isolate *isolate, const std::string &value, const int64_t &default_value);
std::string extract_string(Isolate *isolate, const std::string &value, const std::string &default_value);
void load_plugins();
void plugin_log(const std::string &message);
} // namespace openrasp

ZEND_BEGIN_MODULE_GLOBALS(openrasp_v8)
openrasp::Isolate *isolate = nullptr;
ZEND_END_MODULE_GLOBALS(openrasp_v8)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8);
PHP_GSHUTDOWN_FUNCTION(openrasp_v8);
PHP_MINIT_FUNCTION(openrasp_v8);
PHP_MSHUTDOWN_FUNCTION(openrasp_v8);
PHP_RINIT_FUNCTION(openrasp_v8);

#define OPENRASP_V8_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_v8, v)

#endif /* OPENRASP_v8_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
