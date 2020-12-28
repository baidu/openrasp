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

#include "file_put_webshell_object.h"

namespace openrasp
{
namespace data
{

FilePutWebshellObject::FilePutWebshellObject(zval *file, zval *content, bool use_include_path)
{
    this->file = file;
    this->content = content;
    if (nullptr != file && Z_TYPE_P(file) == IS_STRING && Z_STRLEN_P(file) > 0)
    {
        realpath = openrasp_real_path(Z_STRVAL_P(file), Z_STRLEN_P(file), use_include_path, WRITING);
    }
}
bool FilePutWebshellObject::is_valid() const
{
    return nullptr != content &&
           Z_TYPE_P(content) == IS_STRING &&
           Z_STRLEN_P(content) > 0 &&
           !realpath.empty();
}
OpenRASPCheckType FilePutWebshellObject::get_builtin_check_type() const
{
    return WEBSHELL_FILE_PUT_CONTENTS;
}
void FilePutWebshellObject::fill_json_with_params(JsonReader &j) const
{
    j.write_string({"attack_params", "name"}, std::string(Z_STRVAL_P(file), Z_STRLEN_P(file)));
    j.write_string({"attack_params", "realpath"}, realpath);
    j.write_string({"plugin_message"}, "WebShell activity - Detected file dropper backdoor");
}
std::vector<zval *> FilePutWebshellObject::get_zval_ptrs() const
{
    return {file, content};
}

} // namespace data

} // namespace openrasp