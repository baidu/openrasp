/*
 * Copyright 2017-2018 Baidu Inc.
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

void pre_global_move_uploaded_file(INTERNAL_FUNCTION_PARAMETERS)
{
    zval **name, **dest;
    int argc = MIN(2, ZEND_NUM_ARGS());
    if (!openrasp_check_type_ignored(ZEND_STRL("fileUpload") TSRMLS_CC) &&
        argc == 2 &&
        SG(rfc1867_uploaded_files) != NULL &&
        zend_get_parameters_ex(argc, &name, &dest) == SUCCESS &&
        Z_TYPE_PP(name) == IS_STRING &&
        Z_TYPE_PP(dest) == IS_STRING &&
        zend_hash_exists(SG(rfc1867_uploaded_files), Z_STRVAL_PP(name), Z_STRLEN_PP(name) + 1) &&
        (PG(http_globals)[TRACK_VARS_FILES] || zend_is_auto_global(ZEND_STRL("_FILES") TSRMLS_CC)))
    {
        zval **realname = nullptr;
        HashTable *ht = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_FILES]);
        for (zend_hash_internal_pointer_reset(ht);
             zend_hash_has_more_elements(ht) == SUCCESS;
             zend_hash_move_forward(ht))
        {
            zval **file, **tmp_name;
            if (zend_hash_get_current_data(ht, (void **)&file) != SUCCESS ||
                Z_TYPE_PP(file) != IS_ARRAY ||
                zend_hash_find(Z_ARRVAL_PP(file), ZEND_STRS("tmp_name"), (void **)&tmp_name) != SUCCESS ||
                Z_TYPE_PP(tmp_name) != IS_STRING ||
                zend_binary_strcmp(Z_STRVAL_PP(tmp_name), Z_STRLEN_PP(tmp_name), Z_STRVAL_PP(name), Z_STRLEN_PP(name)) != 0)
            {
                continue;
            }
            if (zend_hash_find(Z_ARRVAL_PP(file), ZEND_STRS("name"), (void **)&realname) == SUCCESS)
            {
                break;
            }
        }
        if (!realname)
        {
            realname = dest;
        }
        php_stream *stream = php_stream_open_wrapper(Z_STRVAL_PP(name), "rb", 0, NULL);
        if (stream)
        {
            char *contents;
            int len = php_stream_copy_to_mem(stream, &contents, 4 * 1024, 0);
            if (len > 0)
            {
                zval *params;
                MAKE_STD_ZVAL(params);
                array_init(params);
                add_assoc_zval(params, "filename", *realname);
                Z_ADDREF_PP(realname);
                add_assoc_stringl(params, "content", contents, MIN(len, 4 * 1024), 0);
                check("fileUpload", params TSRMLS_CC);
            }
        }
    }
}