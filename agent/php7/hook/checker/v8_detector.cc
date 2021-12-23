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

#include "v8_detector.h"
#include "openrasp_v8.h"

namespace openrasp
{
namespace checker
{

bool V8Detector::pretreat() const
{
    if (nullptr == isolate)
    {
        return false;
    }
    if (!v8_material.is_valid())
    {
        return false;
    }
    return true;
}

CheckResult V8Detector::check()
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto params = v8::Object::New(isolate);
    v8_material.fill_object_2b_checked(isolate, params);
    CheckResult check_result = Check(isolate, openrasp::NewV8String(isolate, CheckTypeTransfer::instance().type_to_name(v8_material.get_v8_check_type())), params, timeout);
    return check_result;
}

V8Detector::V8Detector(const openrasp::data::V8Material &v8_material, openrasp::LRU<std::string, bool> &lru, openrasp::Isolate *isolate, int timeout, bool canBlock)
    : v8_material(v8_material), lru(lru), isolate(isolate), timeout(timeout), canBlock(canBlock)
{
}

void V8Detector::run()
{
    if (!pretreat())
    {
        return;
    }
    std::string lru_ley = v8_material.build_lru_key();
    if (!lru_ley.empty() &&
        lru.contains(lru_ley))
    {
        return;
    }
    CheckResult cr = check();
    if (kNoCache == cr)
    {
        return;
    }
    else if (kCache == cr)
    {
        lru.set(lru_ley, true);
    }
    else if (kBlock == cr && canBlock)
    {
        block_handle();
    }
}

} // namespace checker

} // namespace openrasp