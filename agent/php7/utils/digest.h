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

#ifndef _OPENRASP_AGENT_UTILS_DIGEST_H_
#define _OPENRASP_AGENT_UTILS_DIGEST_H_

#include <string>
namespace openrasp
{

/**
 * Same as 'md5sum' command, returned size is always 32.
 */
std::string md5sum(const void *dat, size_t len);

/**
 * Return Calculated raw result(always little-endian), the size is always 16.
 * @out Output result.
 */
void md5bin(const void *dat, size_t len, unsigned char out[16]);

} // namespace openrasp

#endif
