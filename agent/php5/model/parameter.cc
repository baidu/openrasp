/*
 * Copyright 2017-2020 Baidu Inc.
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

#include "parameter.h"
#include "openrasp_utils.h"

#ifdef snprintf
#undef snprintf
#endif

#include "utils/json_reader.h"

namespace openrasp
{
namespace request
{
Parameter::MultipartFile::MultipartFile(const std::string &filename, const std::string &tmpname)
{
    this->filename = filename;
    this->tmpname = tmpname;
}
Parameter::MultipartFile::~MultipartFile()
{
}
void Parameter::MultipartFile::set_filename(const std::string &filename)
{
    this->filename = filename;
}
void Parameter::MultipartFile::set_tmpname(const std::string &tmpname)
{
    this->tmpname = tmpname;
}
std::string Parameter::MultipartFile::get_filename() const
{
    return filename;
}
std::string Parameter::MultipartFile::get_tmpname() const
{
    return tmpname;
}

Parameter::Parameter(/* args */)
{
}

Parameter::~Parameter()
{
}

std::string Parameter::get_form_str() const
{
    return form_str.empty() ? "{}" : form_str;
}

std::string Parameter::get_json_str() const
{
    return json_str.empty() ? "{}" : json_str;
}

std::string Parameter::get_multipart_str()
{
    if (multipart_str.empty())
    {
        JsonReader j;
        std::map<std::string, std::string> name_filename;
        for (auto iter = files.begin(); iter != files.end(); iter++)
        {
            std::string name;
            for (const std::string &item : iter->first)
            {
                name.append(item);
            }
            name_filename.insert({name, iter->second.get_filename()});
        }
        j.write_map_to_array({}, "name", "filename", name_filename);
        multipart_str = j.dump();
    }
    return multipart_str;
}

bool Parameter::fetch_fileinfo_by_tmpname(const std::string &tmpname, std::string &name, std::string &filename) const
{
    for (auto iter = files.begin(); iter != files.end(); iter++)
    {
        if (tmpname == iter->second.get_tmpname())
        {
            name.clear();
            for (const std::string &item : iter->first)
            {
                name.append(item);
            }
            filename = iter->second.get_filename();
            return true;
        }
    }
    return false;
}

std::string Parameter::get_body() const
{
    return body_str;
}

bool Parameter::get_initialized() const
{
    return initialized;
}

void Parameter::set_initialized(bool initialized)
{
    this->initialized = initialized;
}

void Parameter::clear()
{
    form_str.clear();
    json_str.clear();
    body_str.clear();
    files.clear();
    multipart_str.clear();
    initialized = false;
}

//the functions below is different in PHP5 and PHP7
void Parameter::update_body_str(size_t body_len)
{
    TSRMLS_FETCH();
    char *body = nullptr;
    if ((body = fetch_request_body(body_len TSRMLS_CC)) != nullptr)
    {
        body_str = std::string(body);
        efree(body);
    }
}

void Parameter::update_json_str()
{
    TSRMLS_FETCH();
    json_str = "{}";
    char *body = nullptr;
    if ((body = fetch_request_body(PHP_STREAM_COPY_ALL TSRMLS_CC)) != nullptr)
    {
        if (strcmp(body, "") != 0)
        {
            JsonReader json_body(body);
            if (!json_body.has_error())
            {
                json_str = std::string(body);
            }
        }
        efree(body);
    }
}

void Parameter::update_form_str()
{
    TSRMLS_FETCH();
    zval *http_post = fetch_http_globals(TRACK_VARS_POST TSRMLS_CC);
    if (http_post &&
        IS_ARRAY == Z_TYPE_P(http_post) &&
        zend_hash_num_elements(Z_ARRVAL_P(http_post)) > 0)
    {
        form_str = json_encode_from_zval(http_post TSRMLS_CC);
    }
}

void Parameter::update_multipart_files()
{
    TSRMLS_FETCH();
    zval *http_files = fetch_http_globals(TRACK_VARS_FILES TSRMLS_CC);
    if (http_files &&
        IS_ARRAY == Z_TYPE_P(http_files) &&
        zend_hash_num_elements(Z_ARRVAL_P(http_files)) > 0)
    {
        std::vector<std::string> keys;
        HashTable *ht = Z_ARRVAL_P(http_files);
        for (zend_hash_internal_pointer_reset(ht);
             zend_hash_has_more_elements(ht) == SUCCESS;
             zend_hash_move_forward(ht))
        {
            char *key;
            ulong idx;
            int type;
            type = zend_hash_get_current_key(ht, &key, &idx, 0);
            if (type == HASH_KEY_NON_EXISTENT)
            {
                continue;
            }
            zval **file_value;
            zval **name_value;
            zval **tmp_name_value;
            if (zend_hash_get_current_data(ht, (void **)&file_value) != SUCCESS)
            {
                continue;
            }
            if (Z_TYPE_PP(file_value) != IS_ARRAY)
            {
                continue;
            }
            if (zend_hash_find(Z_ARRVAL_PP(file_value), ZEND_STRS("name"), (void **)&name_value) == SUCCESS &&
                zend_hash_find(Z_ARRVAL_PP(file_value), ZEND_STRS("tmp_name"), (void **)&tmp_name_value) == SUCCESS &&
                Z_TYPE_PP(name_value) == Z_TYPE_PP(tmp_name_value))
            {
                if (type == HASH_KEY_IS_STRING)
                {
                    keys.push_back(std::string(key));
                    recursive_restore_files(keys, *name_value, *tmp_name_value);
                    keys.pop_back();
                }
                else if (type == HASH_KEY_IS_LONG)
                {
                    long actual = idx;
                    keys.push_back(std::to_string(actual));
                    recursive_restore_files(keys, *name_value, *tmp_name_value);
                    keys.pop_back();
                }
            }
        }
    }
}

void Parameter::recursive_restore_files(std::vector<std::string> &keys, zval *name, zval *tmp_name)
{
    if (nullptr == name || nullptr == tmp_name || Z_TYPE_P(name) != Z_TYPE_P(tmp_name))
    {
        return;
    }
    if (Z_TYPE_P(name) == IS_STRING)
    {
        files.insert({keys, {Z_STRVAL_P(name), Z_STRVAL_P(tmp_name)}});
    }
    else if (Z_TYPE_P(name) == IS_ARRAY)
    {
        if (zend_hash_num_elements(Z_ARRVAL_P(name)) > 0)
        {
            HashTable *ht = Z_ARRVAL_P(name);
            for (zend_hash_internal_pointer_reset(ht);
                 zend_hash_has_more_elements(ht) == SUCCESS;
                 zend_hash_move_forward(ht))
            {
                char *key;
                ulong idx;
                int type;
                type = zend_hash_get_current_key(ht, &key, &idx, 0);
                if (type == HASH_KEY_NON_EXISTENT)
                {
                    continue;
                }
                zval **name_value;
                zval **tmp_name_value;
                if (zend_hash_get_current_data(ht, (void **)&name_value) != SUCCESS)
                {
                    continue;
                }
                if (type == HASH_KEY_IS_STRING)
                {
                    if (zend_hash_find(Z_ARRVAL_P(tmp_name), key, strlen(key) + 1, (void **)&tmp_name_value) == SUCCESS &&
                        Z_TYPE_PP(name_value) == Z_TYPE_PP(tmp_name_value))
                    {
                        keys.push_back("[" + std::string(key) + "]");
                        recursive_restore_files(keys, *name_value, *tmp_name_value);
                        keys.pop_back();
                    }
                }
                else if (type == HASH_KEY_IS_LONG)
                {
                    if (zend_hash_index_find(Z_ARRVAL_P(tmp_name), idx, (void **)&tmp_name_value) == SUCCESS &&
                        Z_TYPE_PP(name_value) == Z_TYPE_PP(tmp_name_value))
                    {
                        long actual = idx;
                        keys.push_back("[" + std::to_string(actual) + "]");
                        recursive_restore_files(keys, *name_value, *tmp_name_value);
                        keys.pop_back();
                    }
                }
            }
        }
    }
}
} // namespace request

} // namespace openrasp