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

#include "hook/data/fileupload_object.h"
#include "hook/checker/v8_detector.h"
#include "openrasp_hook.h"
#include "openrasp_v8.h"

/**
 * fileupload相关hook点
 */
PRE_HOOK_FUNCTION(move_uploaded_file, FILE_UPLOAD);

void pre_global_move_uploaded_file_FILE_UPLOAD(OPENRASP_INTERNAL_FUNCTION_PARAMETERS)
{
    zval *path = nullptr;
    zval *new_path = nullptr;
    if (!SG(rfc1867_uploaded_files))
    {
        return;
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &path, &new_path) == FAILURE)
    {
        return;
    }
    if (path != nullptr &&
        Z_TYPE_P(path) == IS_STRING &&
        zend_hash_exists(SG(rfc1867_uploaded_files), Z_STRVAL_P(path), Z_STRLEN_P(path) + 1))
    {
        std::string file_content;
        php_stream *stream = php_stream_open_wrapper(Z_STRVAL_P(path), "rb", 0, nullptr);
        if (stream)
        {
            char *contents = nullptr;
            int len = php_stream_copy_to_mem(stream, &contents, 4 * 1024, 0);
            file_content = std::string(contents, len);
            if (nullptr != contents)
            {
                efree(contents);
            }
            php_stream_close(stream);
        }
        openrasp::data::FileuploadObject fileupload_obj(OPENRASP_G(request).get_parameter(), path, new_path, file_content);
        openrasp::checker::V8Detector v8_detector(fileupload_obj, OPENRASP_HOOK_G(lru), OPENRASP_V8_G(isolate), OPENRASP_CONFIG(plugin.timeout.millis));
        v8_detector.run();
    }
}