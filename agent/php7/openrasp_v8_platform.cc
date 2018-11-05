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

#include "openrasp_v8_bundle.h"

namespace openrasp
{
using namespace std;

std::mutex Platform::mtx;
v8::Platform *Platform::platform = nullptr;

void Platform::Initialize()
{
    lock_guard<mutex> lock(mtx);
    if (!platform)
    {
        platform = v8::platform::CreateDefaultPlatform(1);
        v8::V8::InitializePlatform(platform);
    }
}

void Platform::Shutdown()
{
    lock_guard<mutex> lock(mtx);
    if (platform)
    {
        v8::V8::ShutdownPlatform();
        delete platform;
        platform = nullptr;
    }
}
} // namespace openrasp