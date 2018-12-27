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

#pragma once

#include "openrasp.h"
#include "openrasp_log.h"
extern "C"
{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_main.h"
#include "php_output.h"
}

ZEND_BEGIN_MODULE_GLOBALS(openrasp_output_detect)
bool output_detect;
ZEND_END_MODULE_GLOBALS(openrasp_output_detect)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_output_detect);

#ifdef ZTS
#define OUTPUT_G(v) TSRMG(openrasp_output_detect_globals_id, zend_openrasp_output_detect_globals *, v)
#else
#define OUTPUT_G(v) (openrasp_output_detect_globals.v)
#endif

PHP_MINIT_FUNCTION(openrasp_output_detect);
PHP_RINIT_FUNCTION(openrasp_output_detect);
void openrasp_detect_output(INTERNAL_FUNCTION_PARAMETERS);