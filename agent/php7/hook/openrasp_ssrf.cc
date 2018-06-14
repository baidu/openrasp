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

extern "C" {
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
bool pre_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval args[]);
void post_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval args[]);
OPENRASP_HOOK_FUNCTION(curl_exec, ssrf)
{
    bool type_ignored = openrasp_check_type_ignored(ZEND_STRL("ssrf"));
    zval origin_url, function_name;
    zval *zid = nullptr, *opt = nullptr;
    bool skip_hook = false;
    zval args[2];
    ZVAL_STRING(&function_name, "curl_getinfo");
    ZVAL_NULL(&origin_url);
    if (!type_ignored)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE)
        {
            skip_hook = true;
        }
        if ((opt = zend_get_constant_str(ZEND_STRL("CURLINFO_EFFECTIVE_URL"))) == nullptr)
        {
            skip_hook = true;
        }
        args[0] = *zid;
        args[1] = *opt;
        if (!skip_hook)
        {
            skip_hook = pre_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, "ssrf", &function_name, opt, &origin_url, args);
        }
    }
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    if (!type_ignored && !skip_hook)
    {
        post_global_curl_exec_ssrf(INTERNAL_FUNCTION_PARAM_PASSTHRU, "ssrf", &function_name, opt, &origin_url, args);
    }
    zval_ptr_dtor(&origin_url);
    zval_ptr_dtor(&function_name);
}

bool pre_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval args[])
{
    if (call_user_function(EG(function_table), NULL, function_name, origin_url, 2, args) != SUCCESS ||
        Z_TYPE_P(origin_url) != IS_STRING)
    {
        return true;
    }
    {
        zval params;
        array_init(&params);
        add_assoc_zval(&params, "url", origin_url);
        Z_TRY_ADDREF_P(origin_url);
        add_assoc_string(&params, "function", const_cast<char *>("curl_exec"));
        php_url *url = php_url_parse_ex(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url));
        if (url && url->host)
        {
            add_assoc_str(&params, "hostname", (zend_string_init(url->host, strlen(url->host), 0)));
        }
        else
        {
            add_assoc_string(&params, "hostname", "");
        }
        zval ip_arr;
        array_init(&ip_arr);
        if (url)
        {
            if (url->host)
            {
                struct hostent *hp;
                struct in_addr in;
                int i;
                hp = gethostbyname(url->host);
                if (hp != NULL && hp->h_addr_list != NULL)
                {
                    for (i = 0; hp->h_addr_list[i] != 0; i++)
                    {
                        in = *(struct in_addr *)hp->h_addr_list[i];
                        add_next_index_string(&ip_arr, inet_ntoa(in));
                    }
                }
            }
            php_url_free(url);
        }
        add_assoc_zval(&params, "ip", &ip_arr);
        check(check_type, &params);
    }
    return false;
}

void post_global_curl_exec_ssrf(OPENRASP_INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval args[])
{
    zval effective_url;
    ZVAL_NULL(&effective_url);
    if (call_user_function(EG(function_table), NULL, function_name, &effective_url, 2, args) != SUCCESS &&
        Z_TYPE(effective_url) != IS_STRING &&
        (strncasecmp(Z_STRVAL(effective_url), "file", 4) == 0 || strncasecmp(Z_STRVAL(effective_url), "scp", 3) == 0) &&
        strcmp(Z_STRVAL(effective_url), Z_STRVAL_P(origin_url)) != 0)
    {
        zval attack_params;
        ZVAL_STRING(&attack_params, Z_STRVAL(effective_url));
        zval plugin_message;
        ZVAL_STR(&plugin_message, strpprintf(0, _("Detected SSRF via 302 redirect, effective url is %s"), Z_STRVAL(effective_url)));
        openrasp_buildin_php_risk_handle(1, check_type, 100, &attack_params, &plugin_message);
    }
    zval_ptr_dtor(&effective_url);
    return;
}