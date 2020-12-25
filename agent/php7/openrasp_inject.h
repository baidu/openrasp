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

#ifndef OPENRASP_INJECT_H
#define OPENRASP_INJECT_H

#include "openrasp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "Zend/zend_API.h"
#include "ext/standard/php_string.h"
#ifdef __cplusplus
}
#endif

ZEND_BEGIN_MODULE_GLOBALS(openrasp_inject)

ZEND_END_MODULE_GLOBALS(openrasp_inject)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_inject);

#define OPENRASP_INJECT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_inject, v)

// #ifdef ZTS
// #define OPENRASP_INJECT_G(v) TSRMG(openrasp_inject_globals_id, zend_openrasp_inject_globals *, v)
// #define OPENRASP_INJECT_GP() ((zend_openrasp_inject_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_inject_globals_id)])
// #else
// #define OPENRASP_INJECT_G(v) (openrasp_inject_globals.v)
// #define OPENRASP_INJECT_GP() (&openrasp_inject_globals)
// #endif

PHP_MINIT_FUNCTION(openrasp_inject);
PHP_MSHUTDOWN_FUNCTION(openrasp_inject);
PHP_RINIT_FUNCTION(openrasp_inject);
PHP_RSHUTDOWN_FUNCTION(openrasp_inject);


#endif