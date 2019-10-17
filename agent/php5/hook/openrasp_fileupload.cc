/*
 * Copyright 2017-2019 Baidu Inc.
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

#include "openrasp_hook.h"
#include "openrasp_v8.h"

/**
 * fileupload相关hook点
 */
PRE_HOOK_FUNCTION(move_uploaded_file, FILE_UPLOAD);

void pre_global_move_uploaded_file_FILE_UPLOAD(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    char *path = nullptr;
    char *new_path = nullptr;
    int path_len = 0;
    int new_path_len = 0;

    if (!SG(rfc1867_uploaded_files))
    {
        return;
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &path, &path_len, &new_path, &new_path_len) == FAILURE)
    {
        return;
    }
    if (zend_hash_exists(SG(rfc1867_uploaded_files), path, path_len + 1) &&
        (PG(http_globals)[TRACK_VARS_FILES] || zend_is_auto_global(ZEND_STRL("_FILES") TSRMLS_CC)))
    {
        zval **realname = nullptr;
        std::string form_data_name;
        std::string realname_str;
        HashTable *ht = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_FILES]);
        for (zend_hash_internal_pointer_reset(ht);
             zend_hash_has_more_elements(ht) == SUCCESS;
             zend_hash_move_forward(ht))
        {
            char *key = nullptr;
            ulong idx;
            int type = 0;
            zval **file, **tmp_name;
            type = zend_hash_get_current_key(ht, &key, &idx, 0);
            if (type == HASH_KEY_NON_EXISTENT)
            {
                continue;
            }
            else if (type == HASH_KEY_IS_STRING)
            {
                form_data_name = std::string(key);
            }
            else if (type == HASH_KEY_IS_LONG)
            {
                long actual = idx;
                form_data_name = std::to_string(actual);
            }
            if (zend_hash_get_current_data(ht, (void **)&file) == SUCCESS &&
                Z_TYPE_PP(file) == IS_ARRAY &&
                zend_hash_find(Z_ARRVAL_PP(file), ZEND_STRS("tmp_name"), (void **)&tmp_name) == SUCCESS)
            {
                if (Z_TYPE_PP(tmp_name) == IS_STRING)
                {
                    if (zend_binary_strcmp(Z_STRVAL_PP(tmp_name), Z_STRLEN_PP(tmp_name), path, path_len) == 0)
                    {
                        if (zend_hash_find(Z_ARRVAL_PP(file), ZEND_STRS("name"), (void **)&realname) == SUCCESS &&
                            IS_STRING == Z_TYPE_PP(realname))
                        {
                            realname_str = std::string(Z_STRVAL_PP(realname), Z_STRLEN_PP(realname));
                            break;
                        }
                    }
                }
                else if (Z_TYPE_PP(tmp_name) == IS_ARRAY)
                {
                    char *tmp_name_key = nullptr;
                    ulong tmp_name_idx;
                    int tmp_name_type = 0;
                    bool found = false;
                    HashTable *tmp_name_ht = Z_ARRVAL_PP(tmp_name);
                    for (zend_hash_internal_pointer_reset(tmp_name_ht);
                         zend_hash_has_more_elements(tmp_name_ht) == SUCCESS;
                         zend_hash_move_forward(tmp_name_ht))
                    {
                        tmp_name_type = zend_hash_get_current_key(tmp_name_ht, &tmp_name_key, &tmp_name_idx, 0);
                        if (tmp_name_type == HASH_KEY_NON_EXISTENT)
                        {
                            continue;
                        }
                        zval **tmp_name_file;
                        if (zend_hash_get_current_data(tmp_name_ht, (void **)&tmp_name_file) == SUCCESS &&
                            Z_TYPE_PP(tmp_name_file) == IS_STRING &&
                            zend_binary_strcmp(Z_STRVAL_PP(tmp_name_file), Z_STRLEN_PP(tmp_name_file), path, path_len) == 0)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                    {
                        if (zend_hash_find(Z_ARRVAL_PP(file), ZEND_STRS("name"), (void **)&realname) == SUCCESS &&
                            IS_ARRAY == Z_TYPE_PP(realname))
                        {
                            zval **realname_item = nullptr;
                            if (tmp_name_type == HASH_KEY_IS_STRING)
                            {
                                if (zend_hash_find(Z_ARRVAL_PP(realname), tmp_name_key, strlen(tmp_name_key) + 1, (void **)&realname_item) == SUCCESS &&
                                    IS_STRING == Z_TYPE_PP(realname_item))
                                {
                                    realname_str = std::string(Z_STRVAL_PP(realname_item), Z_STRLEN_PP(realname_item));
                                    break;
                                }
                            }
                            else if (tmp_name_type == HASH_KEY_IS_LONG)
                            {
                                if (zend_hash_index_find(Z_ARRVAL_PP(realname), tmp_name_idx, (void **)&realname_item) == SUCCESS &&
                                    IS_STRING == Z_TYPE_PP(realname_item))
                                {
                                    realname_str = std::string(Z_STRVAL_PP(realname_item), Z_STRLEN_PP(realname_item));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        php_stream *stream = php_stream_open_wrapper(path, "rb", 0, nullptr);
        if (stream)
        {
            char *contents = nullptr;
            int len = php_stream_copy_to_mem(stream, &contents, 4 * 1024, 0);
            php_stream_close(stream);
            if (len > 0)
            {
                openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
                openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
                std::string real_dest = openrasp_real_path(new_path, new_path_len, 0, WRITING TSRMLS_CC);
                if (isolate)
                {
                    v8::HandleScope handle_scope(isolate);
                    auto params = v8::Object::New(isolate);
                    params->Set(openrasp::NewV8String(isolate, "name"), openrasp::NewV8String(isolate, form_data_name));
                    params->Set(openrasp::NewV8String(isolate, "filename"), openrasp::NewV8String(isolate, realname_str));
                    params->Set(openrasp::NewV8String(isolate, "dest_path"), openrasp::NewV8String(isolate, new_path, new_path_len));
                    params->Set(openrasp::NewV8String(isolate, "dest_realpath"), openrasp::NewV8String(isolate, real_dest));
                    params->Set(openrasp::NewV8String(isolate, "content"), openrasp::NewV8String(isolate, contents, MIN(len, 4 * 1024)));
                    check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
                }
                efree(contents);
                if (check_result == openrasp::CheckResult::kBlock)
                {
                    handle_block(TSRMLS_C);
                }
            }
        }
    }
}