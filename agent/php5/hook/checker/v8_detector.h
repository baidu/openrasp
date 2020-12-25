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

#pragma once

#include "detector.h"
#include "hook/data/v8_material.h"
#include "openrasp_hook.h"

namespace openrasp
{
namespace checker
{

class V8Detector : public Detector
{
protected:
    const openrasp::data::V8Material &v8_material;
    openrasp::LRU<std::string, bool> &lru;
    openrasp::Isolate *isolate = nullptr;
    int timeout = 100;
    bool canBlock = true;

    virtual bool pretreat() const;
    virtual CheckResult check();

public:
    V8Detector(const openrasp::data::V8Material &v8_material, openrasp::LRU<std::string, bool> &lru, openrasp::Isolate *isolate, int timeout, bool canblock = true);
    virtual void run();
};

} // namespace checker

} // namespace openrasp