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

#include "zend_ref_item.h"

namespace openrasp
{
namespace request
{

ZendRefItem::ZendRefItem(int id, const std::string &name)
{
    this->id = id;
    this->name = name;
}

void ZendRefItem::set_id(int id)
{
    this->id = id;
}
int ZendRefItem::get_id() const
{
    return id;
}
void ZendRefItem::set_name(const std::string &name)
{
    this->name = name;
}
std::string ZendRefItem::get_name() const
{
    return name;
}

} // namespace request

} // namespace openrasp