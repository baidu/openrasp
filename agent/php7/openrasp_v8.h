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

#ifndef OPENRASP_V8_H
#define OPENRASP_V8_H

#include "openrasp.h"
#include "openrasp_v8_bundle.h"

namespace openrasp
{
class openrasp_v8_process_globals
{
public:
  Snapshot *snapshot_blob = nullptr;
  std::mutex mtx;
  std::string plugin_config;
  std::vector<PluginFile> plugin_src_list;
};
extern openrasp_v8_process_globals process_globals;
v8::Local<v8::Value> NewV8ValueFromZval(v8::Isolate *isolate, zval *val);
void load_plugins();
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
