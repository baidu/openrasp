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

#include "rename_object.h"

namespace openrasp
{
namespace data
{

RenameObject::RenameObject(zval *source, zval *target, bool plugin_filter)
{
    this->source = source;
    this->target = target;
    this->plugin_filter = plugin_filter;
    if (nullptr != source && Z_TYPE_P(source) == IS_STRING && Z_STRLEN_P(source) > 0)
    {
        source_realpath = openrasp_real_path(Z_STRVAL_P(source), Z_STRLEN_P(source), false, RENAMESRC);
    }
    if (nullptr != target && Z_TYPE_P(target) == IS_STRING && Z_STRLEN_P(target) > 0)
    {
        target_realpath = openrasp_real_path(Z_STRVAL_P(target), Z_STRLEN_P(target), false, RENAMEDEST);
    }
}
bool RenameObject::is_valid() const
{
    if (source_realpath.empty() || target_realpath.empty())
    {
        return false;
    }

    std::string src_protocol = fetch_possible_protocol(source_realpath.c_str());
    std::string tgt_protocol = fetch_possible_protocol(target_realpath.c_str());
    if (!src_protocol.empty() && !tgt_protocol.empty())
    {
        if (!openrasp::case_insens_equal(src_protocol, tgt_protocol))
        {
            return false;
        }
    }
    else if (src_protocol.empty() && tgt_protocol.empty())
    {
        if (plugin_filter)
        {
            struct stat src_sb;
            if (stat(source_realpath.c_str(), &src_sb) == 0 && (src_sb.st_mode & S_IFDIR) != 0)
            {
                return false;
            }
            else
            {
                struct stat tgt_sb;
                if (stat(target_realpath.c_str(), &tgt_sb) == 0)
                {
                    if ((tgt_sb.st_mode & S_IFDIR) != 0)
                    {
                        return false;
                    }
                }
                else
                {
                    if (end_with(target_realpath, std::string(1, DEFAULT_SLASH)))
                    {
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

//v8
std::string RenameObject::build_lru_key() const
{
    return CheckTypeTransfer::instance().type_to_name(get_v8_check_type()) + source_realpath + target_realpath;
}
OpenRASPCheckType RenameObject::get_v8_check_type() const
{
    return RENAME;
}
void RenameObject::fill_object_2b_checked(Isolate *isolate, v8::Local<v8::Object> params) const
{
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    params->Set(context, openrasp::NewV8String(isolate, "source"), openrasp::NewV8String(isolate, source_realpath)).IsJust();
    params->Set(context, openrasp::NewV8String(isolate, "dest"), openrasp::NewV8String(isolate, target_realpath)).IsJust();
}

} // namespace data

} // namespace openrasp