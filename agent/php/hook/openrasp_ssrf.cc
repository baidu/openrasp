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

extern "C"
{
#ifdef PHP_WIN32
# include "win32/inet.h"
# include <winsock2.h>
# include <windows.h>
# include <Ws2tcpip.h>
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

int pre_global_curl_exec(INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args)
{
    zval **zid;

    int argc = MIN(1, ZEND_NUM_ARGS());
    if (openrasp_check_type_ignored(ZEND_STRL("ssrf") TSRMLS_CC) ||
        argc <= 0 ||
        zend_get_parameters_ex(argc, &zid) != SUCCESS ||
        Z_TYPE_PP(zid) != IS_RESOURCE)
    {
        return 1;
    }
    if (!zend_get_constant(ZEND_STRL("CURLINFO_EFFECTIVE_URL"), opt TSRMLS_CC))
    {
        return 1;
    }
    args[0] = *zid;
    args[1] = opt;
    if (call_user_function(EG(function_table), NULL, function_name, origin_url, 2, args TSRMLS_CC) != SUCCESS ||
        Z_TYPE_P(origin_url) != IS_STRING)
    {
        return 1;
    }
    {
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        add_assoc_zval(params, "url", origin_url);
        Z_ADDREF_P(origin_url);
        add_assoc_string(params, "function", estrdup("curl_exec"), 0);
        php_url *url = php_url_parse_ex(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url));
        add_assoc_string(params, "hostname", (url && url->host) ? estrdup(url->host) : estrdup(""), 0);
        zval *ip_arr = NULL;
        MAKE_STD_ZVAL(ip_arr);
        array_init(ip_arr);
        if (url) 
        {
            if (url->host)
            {
                struct hostent *hp;
                struct in_addr in;
                int i;
                hp = gethostbyname(url->host);
                if (hp != NULL && hp->h_addr_list != NULL) {
                    for (i = 0 ; hp->h_addr_list[i] != 0 ; i++) {
                        in = *(struct in_addr *) hp->h_addr_list[i];
                        add_next_index_string(ip_arr, inet_ntoa(in), 1);
                    }
                }
            }
            php_url_free(url);
        }
        add_assoc_zval(params, "ip", ip_arr);
        check("ssrf", params TSRMLS_CC);
    }
    return 0;
}

void post_global_curl_exec(INTERNAL_FUNCTION_PARAMETERS, zval *function_name, zval *opt, zval *origin_url, zval **args)
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
        ZVAL_STRING(attack_params, Z_STRVAL(effective_url), 1);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        char *message_str = NULL;
        spprintf(&message_str, 0, _("Detected SSRF via 302 redirect, effective url is %s"), Z_STRVAL(effective_url));
        ZVAL_STRING(plugin_message, message_str, 1);
        efree(message_str);
        openrasp_buildin_php_risk_handle(1, "ssrf", 100, attack_params, plugin_message TSRMLS_CC);       
    }
    zval_dtor(&effective_url);
    return;
}