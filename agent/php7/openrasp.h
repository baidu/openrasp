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

#ifndef OPENRASP_H
#define OPENRASP_H

#include "php_openrasp.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(STRING) gettext(STRING)
#else
#define _(STRING) (STRING)
#endif

#ifdef __cplusplus
}
#endif

#include "openrasp_conf_holder.h"
#include "openrasp_error.h"
#include "model/request.h"

#ifndef ZEND_SHUTDOWN_MODULE_GLOBALS
#ifdef ZTS
#define ZEND_SHUTDOWN_MODULE_GLOBALS(module_name, globals_dtor) \
	ts_free_id(module_name##_globals_id);
#else
#define ZEND_SHUTDOWN_MODULE_GLOBALS(module_name, globals_dtor) \
	globals_dtor(&module_name##_globals);
#endif
#endif

ZEND_BEGIN_MODULE_GLOBALS(openrasp)
openrasp::ConfigHolder config;
openrasp::request::Request request;
ZEND_END_MODULE_GLOBALS(openrasp)

ZEND_EXTERN_MODULE_GLOBALS(openrasp)

#define OPENRASP_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp, v)

// #ifdef ZTS
// #define OPENRASP_G(v) TSRMG(openrasp_globals_id, zend_openrasp_globals *, v)
// #define OPENRASP_GP() ((zend_openrasp_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_globals_id)])
// #else
// #define OPENRASP_G(v) (openrasp_globals.v)
// #define OPENRASP_GP() (&openrasp_globals)
// #endif

#define OPENRASP_CONFIG(key) (OPENRASP_G(config).key)

#ifdef UNLIKELY
#undef UNLIKELY
#endif
#ifdef LIKELY
#undef LIKELY
#endif
#if defined(__GNUC__) || defined(__clang__)
#define UNLIKELY(condition) (__builtin_expect(!!(condition), 0))
#define LIKELY(condition) (__builtin_expect(!!(condition), 1))
#else
#define UNLIKELY(condition) (condition)
#define LIKELY(condition) (condition)
#endif

struct OpenRASPInfo
{
	static const char *PHP_OPENRASP_VERSION; /* Replace with version number for your extension */
};

#endif