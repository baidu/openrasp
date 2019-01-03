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

#include "openrasp_inject.h"
#include "openrasp_ini.h"
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <new>

ZEND_DECLARE_MODULE_GLOBALS(openrasp_inject)
std::vector<char> inject_html;

void openrasp_load_inject_html(TSRMLS_D)
{
    std::vector<char> inject;
    char *path;
    spprintf(&path, 0, "%s%cassets%cinject.html", openrasp_ini.root_dir, DEFAULT_SLASH, DEFAULT_SLASH);
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    efree(path);
    if (file.is_open())
    {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        inject.resize(size);
        file.read(inject.data(), size);
    }
    inject_html = inject;
}

PHP_GINIT_FUNCTION(openrasp_inject)
{
#ifdef ZTS
    new (openrasp_inject_globals) _zend_openrasp_inject_globals;
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp_inject)
{
#ifdef ZTS
    openrasp_inject_globals->~_zend_openrasp_inject_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp_inject)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_inject, PHP_GINIT(openrasp_inject), PHP_GSHUTDOWN(openrasp_inject));
    openrasp_load_inject_html(TSRMLS_C);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_inject)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_inject, PHP_GSHUTDOWN(openrasp_inject));
    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_inject)
{
    {
        auto time_point = std::chrono::steady_clock::now();
        long long nano = time_point.time_since_epoch().count();
        unsigned long hash = zend_inline_hash_func(reinterpret_cast<const char *>(&nano), sizeof(nano));
        spprintf(&OPENRASP_INJECT_G(request_id), 32, "%016lx%016llx", hash, nano);
        char *uuid_header = nullptr;
        int uuid_header_len = spprintf(&uuid_header, 0, "X-Request-ID: %s", OPENRASP_INJECT_G(request_id));
        if (uuid_header)
        {
            sapi_header_line header;
            header.line = uuid_header;
            header.line_len = uuid_header_len;
            header.response_code = 0;
            sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
        }
        efree(uuid_header);
    }
    {
        sapi_header_line header;
        header.line = "X-Protected-By: OpenRASP";
        header.line_len = sizeof("X-Protected-By: OpenRASP") - 1;
        header.response_code = 0;
        sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
    }
    for (const auto &it : OPENRASP_CONFIG(inject.headers))
    {
        sapi_header_line header;
        header.line = const_cast<char *>(it.c_str());
        header.line_len = it.length();
        header.response_code = 0;
        sapi_header_op(SAPI_HEADER_REPLACE, &header TSRMLS_CC);
    }
    return SUCCESS;
}
PHP_RSHUTDOWN_FUNCTION(openrasp_inject)
{
    static const char REQUEST_URI[] = "REQUEST_URI";
    static const ulong REQUEST_URI_HASH = zend_get_hash_value(ZEND_STRS(REQUEST_URI));
    if (inject_html.size())
    {
        bool is_match_inject_prefix = false;
        if (!OPENRASP_CONFIG(inject.urlprefix).empty())
        {
            is_match_inject_prefix = false;
            if (PG(http_globals)[TRACK_VARS_SERVER] || zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
            {
                zval **value;
                if (zend_hash_quick_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRS(REQUEST_URI), REQUEST_URI_HASH, (void **)&value) == SUCCESS &&
                    Z_TYPE_PP(value) == IS_STRING &&
                    strncasecmp(Z_STRVAL_PP(value), OPENRASP_CONFIG(inject.urlprefix).c_str(), OPENRASP_CONFIG(inject.urlprefix).length()) == 0)
                {
                    is_match_inject_prefix = true;
                }
            }
        }
        if (is_match_inject_prefix)
        {
            if (strncasecmp(SG(sapi_headers).mimetype, "text/html", sizeof("text/html") - 1) == 0)
            {
#if PHP_MINOR_VERSION > 3
                php_output_write(inject_html.data(), inject_html.size() TSRMLS_CC);
#else
                php_body_write(inject_html.data(), inject_html.size() TSRMLS_CC);
#endif
            }
        }
    }
    efree(OPENRASP_INJECT_G(request_id));
    return SUCCESS;
}