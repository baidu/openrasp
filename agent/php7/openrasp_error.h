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

#ifndef OPENRASP_ERROR_H
#define OPENRASP_ERROR_H

#include "openrasp.h"

enum openrasp_error_code
{
	FSWATCH_ERROR = 20001,
	LOG_ERROR,
	SHM_ERROR,
	CONFIG_ERROR,
	PLUGIN_ERROR,
	RUNTIME_ERROR,
	REGISTER_ERROR = 20008,
	HEARTBEAT_ERROR,
	LOGCOLLECT_ERROR,
	DEPENDENCY_ERROR = 20015,
	CRASH_ERROR = 200020
};


void openrasp_error(int type, openrasp_error_code code, const char *format, ...);

#endif