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

#ifndef _OPENRASP_BASE_MANAGER_H_
#define _OPENRASP_BASE_MANAGER_H_

#include "openrasp.h"
#include "mm/shm_manager.h"

namespace openrasp
{

#if (PHP_MAJOR_VERSION == 5)
#define TS_FETCH_WRAPPER() TSRMLS_FETCH()
#else
#define TS_FETCH_WRAPPER()
#endif
class ShmManager;

class BaseManager
{

protected:
  static ShmManager sm;

protected:
  bool initialized = false;

public:
  BaseManager();
  virtual bool startup() = 0;
  virtual bool shutdown() = 0;
};

} // namespace openrasp

#endif
