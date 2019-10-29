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
    if (zend_hash_exists(SG(rfc1867_uploaded_files), path, path_len + 1))
    {
        std::string name;
        std::string filename;
        if (OPENRASP_G(request).get_parameter().fetch_fileinfo_by_tmpname(std::string(path, path_len), name, filename))
        {
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
                        params->Set(openrasp::NewV8String(isolate, "name"), openrasp::NewV8String(isolate, name));
                        params->Set(openrasp::NewV8String(isolate, "filename"), openrasp::NewV8String(isolate, filename));
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
}