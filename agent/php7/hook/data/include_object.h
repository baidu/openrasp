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

#include "php_openrasp.h"
#include "v8_material.h"

namespace openrasp
{
namespace data
{

class IncludeObject : public V8Material
{
private:
    //do not efree here
    zval *filename = nullptr;
    std::string realpath;
    const std::string document_root;
    const std::string function;
    std::string param;
    bool plugin_filter = false;
    bool without_protocol = false;

public:
    IncludeObject(zval *filename, const std::string &document_root, const std::string &function, bool plugin_filter, bool without_protocol);
    virtual std::string build_lru_key() const;
    virtual OpenRASPCheckType get_v8_check_type() const;
    virtual bool is_valid() const;
    virtual void fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const;
};

} // namespace data

} // namespace openrasp