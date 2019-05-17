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

extern "C"
{
#ifdef PHP_WIN32
#include "win32/inet.h"
#include <winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#else
#include <netinet/in.h>
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <netdb.h>
#ifdef HAVE_DNS_H
#include <dns.h>
#endif
#endif
}

/**
 * ssrf相关hook点
 */
bool pre_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args);
void post_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args);
OPENRASP_HOOK_FUNCTION(curl_exec, ssrf)
{
    bool type_ignored = openrasp_check_type_ignored(SSRF TSRMLS_CC);
    zval function_name, opt, origin_url, *args[2];
    bool skip_post = true;
    if (!type_ignored)
    {
        INIT_ZVAL(function_name);
        INIT_ZVAL(opt);
        INIT_ZVAL(origin_url);
        ZVAL_STRING(&function_name, "curl_getinfo", 0); // 不需要 zval_dtor

        skip_post = pre_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, SSRF, &function_name, &opt, &origin_url, args);
    }
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    if (!type_ignored)
    {
        if (!skip_post)
        {
            post_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, SSRF, &function_name, &opt, &origin_url, args);
        }

        zval_dtor(&opt);
        zval_dtor(&origin_url);
    }
}

bool pre_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args)
{
    zval **zid;

    int argc = MIN(1, ZEND_NUM_ARGS());
    if (argc <= 0 ||
        zend_get_parameters_ex(argc, &zid) != SUCCESS ||
        Z_TYPE_PP(zid) != IS_RESOURCE)
    {
        return true;
    }
    if (!zend_get_constant(ZEND_STRL("CURLINFO_EFFECTIVE_URL"), opt TSRMLS_CC))
    {
        return true;
    }
    args[0] = *zid;
    args[1] = opt;
    if (call_user_function(EG(function_table), NULL, function_name, origin_url, 2, args TSRMLS_CC) != SUCCESS ||
        Z_TYPE_P(origin_url) != IS_STRING)
    {
        return true;
    }
    std::string url_string(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url));
    std::string scheme;
    std::string host;
    std::string port;
    if (!openrasp_parse_url(url_string, scheme, host, port) || scheme.empty())
    {
        if (!openrasp_parse_url("http://" + url_string, scheme, host, port))
        {
            return false;
        }
    }
    openrasp::Isolate *isolate = OPENRASP_V8_G(isolate);
    if (isolate)
    {
        std::string cache_key;
        openrasp::CheckResult check_result = openrasp::CheckResult::kCache;
        {
            v8::HandleScope handle_scope(isolate);
            auto params = v8::Object::New(isolate);
            params->Set(openrasp::NewV8String(isolate, "url"), openrasp::NewV8String(isolate, Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url)));
            params->Set(openrasp::NewV8String(isolate, "function"), openrasp::NewV8String(isolate, "curl_exec"));
            params->Set(openrasp::NewV8String(isolate, "hostname"), openrasp::NewV8String(isolate, host));
            params->Set(openrasp::NewV8String(isolate, "port"), openrasp::NewV8String(isolate, port));
            auto ip_arr = v8::Array::New(isolate);
            struct hostent *hp = gethostbyname(host.c_str());
            uint32_t ip_sum = 0;
            if (hp && hp->h_addr_list)
            {
                for (int i = 0; hp->h_addr_list[i] != 0; i++)
                {
                    struct in_addr in = *(struct in_addr *)hp->h_addr_list[i];
                    ip_sum += in.s_addr;
                    ip_arr->Set(i, openrasp::NewV8String(isolate, inet_ntoa(in)));
                }
            }
            params->Set(openrasp::NewV8String(isolate, "ip"), ip_arr);
            {
                cache_key = std::string(get_check_type_name(check_type) + std::string(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url)) + std::to_string(ip_sum));
                if (OPENRASP_HOOK_G(lru).contains(cache_key))
                {
                    return false;
                }
            }
            check_result = Check(isolate, openrasp::NewV8String(isolate, get_check_type_name(check_type)), params, OPENRASP_CONFIG(plugin.timeout.millis));
        }
        if (check_result == openrasp::CheckResult::kCache)
        {
            OPENRASP_HOOK_G(lru).set(cache_key, true);
        }
        if (check_result == openrasp::CheckResult::kBlock)
        {
            handle_block(TSRMLS_C);
        }
    }
    return false;
}

void post_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args)
{
    zval effective_url;
    INIT_ZVAL(effective_url);
    if (call_user_function(EG(function_table), NULL, function_name, &effective_url, 2, args TSRMLS_CC) != SUCCESS &&
        Z_TYPE(effective_url) != IS_STRING &&
        (strncasecmp(Z_STRVAL(effective_url), "file", 4) == 0 || strncasecmp(Z_STRVAL(effective_url), "scp", 3) == 0) &&
        strcmp(Z_STRVAL(effective_url), Z_STRVAL_P(origin_url)) != 0)
    {
        zval *attack_params = NULL;
        MAKE_STD_ZVAL(attack_params);
        array_init(attack_params);
        add_assoc_string(attack_params, "url", Z_STRVAL(effective_url), 1);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        char *message_str = NULL;
        spprintf(&message_str, 0, _("SSRF - Detected SSRF via 302 redirect, current effective url is %s"), Z_STRVAL(effective_url));
        ZVAL_STRING(plugin_message, message_str, 1);
        efree(message_str);
        openrasp_buildin_php_risk_handle(AC_BLOCK, check_type, 100, attack_params, plugin_message TSRMLS_CC);
    }
    zval_dtor(&effective_url);
    return;
}