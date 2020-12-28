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

#include <string>

namespace openrasp
{
namespace request
{
class ZendRefItem
{
private:
    /* data */
    int id;
    std::string name;

public:

    ZendRefItem() = default;
    ZendRefItem(int id, const std::string &name);
    void set_id(int id);
    int get_id() const;
    void set_name(const std::string &name);
    std::string get_name() const;
};
} // namespace request

} // namespace openrasp