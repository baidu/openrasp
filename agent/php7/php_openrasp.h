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

#ifndef PHP_OPENRASP_H
#define PHP_OPENRASP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// bug fix, isfinite is not declared
#ifndef HAVE_ISFINITE
#include <cmath>
#define isfinite std::isfinite
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include "php.h"

  extern zend_module_entry openrasp_module_entry;
#define phpext_openrasp_ptr &openrasp_module_entry

#ifdef PHP_WIN32
#define PHP_OPENRASP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_OPENRASP_API __attribute__((visibility("default")))
#else
#define PHP_OPENRASP_API
#endif

#ifdef ZTS
#include "TSRM.h"
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
#ifdef __cplusplus
}
#endif

#ifndef HAVE_ISFINITE
#undef isfinite
#endif

#endif /* PHP_OPENRASP_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
