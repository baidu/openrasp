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

#include "utils/json_reader.h"
#include "check_utils.h"
#include "detector.h"
#include "hook/data/policy_material.h"

namespace openrasp
{
namespace checker
{

class PolicyDetector : public Detector
{
protected:
    const openrasp::data::PolicyMaterial &policy_material;
    JsonReader j;

    virtual bool pretreat() const;
    virtual CheckResult check();
    virtual void log_policy();

public:
    PolicyDetector(const openrasp::data::PolicyMaterial &policy_material);
    virtual void run();
};

} // namespace checker

} // namespace openrasp