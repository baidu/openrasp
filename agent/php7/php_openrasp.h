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

#ifndef PHP_OPENRASP_H
#define PHP_OPENRASP_H

#include "php.h"

extern zend_module_entry openrasp_module_entry;
#define phpext_openrasp_ptr &openrasp_module_entry

#define PHP_OPENRASP_VERSION "0.1.0" /* Replace with version number for your extension */
#define OPENRASP_PHP_VERSION ZEND_TOSTR(PHP_MAJOR_VERSION.PHP_MINOR_VERSION.PHP_RELEASE_VERSION)

#ifdef PHP_WIN32
#	define PHP_OPENRASP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_OPENRASP_API __attribute__ ((visibility("default")))
#else
#	define PHP_OPENRASP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/// BEGIN PHP <= 5.4 ///
#ifndef ZEND_MOD_END
#define ZEND_MOD_END    \
  {                     \
    NULL, NULL, NULL, 0 \
  }
#endif

#ifndef ZVAL_COPY_VALUE
#define ZVAL_COPY_VALUE(z, v)  \
  do                           \
  {                            \
    (z)->value = (v)->value;   \
    Z_TYPE_P(z) = Z_TYPE_P(v); \
  } while (0)
#endif

#ifndef HASH_KEY_NON_EXISTENT
#define HASH_KEY_NON_EXISTENT HASH_KEY_NON_EXISTANT
#endif
/// END PHP <= 5.4 ///

#endif	/* PHP_OPENRASP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
