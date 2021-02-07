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
#include <map>
#include "url.h"
#include "php_openrasp.h"

namespace openrasp
{
namespace request
{
class Parameter
{
public:
    class MultipartFile
    {
    private:
        /* data */
        std::string filename;
        std::string tmpname;

    public:
        MultipartFile(const std::string &filename, const std::string &tmpname);
        virtual ~MultipartFile();
        void set_filename(const std::string &filename);
        void set_tmpname(const std::string &tmpname);
        std::string get_filename() const;
        std::string get_tmpname() const;
    };

private:
    /* data */
    std::string form_str;
    std::string json_str;
    std::string body_str;
    std::string multipart_str;
    std::map<std::vector<std::string>, MultipartFile> files;
    bool initialized = false;

public:
    Parameter(/* args */);
    virtual ~Parameter();
    bool get_initialized() const;
    void set_initialized(bool initialized);
    void update_body_str(size_t body_len);
    void update_json_str();
    void update_form_str();
    void update_multipart_files();
    std::string get_form_str() const;
    std::string get_json_str() const;
    std::string get_multipart_str();
    std::string get_body() const;
    bool fetch_fileinfo_by_tmpname(const std::string &tmpname, std::string &name, std::string &filename) const;
    void clear();
    void recursive_restore_files(std::vector<std::string> &keys, zval *name, zval *tmp_name);
};
} // namespace request

} // namespace openrasp