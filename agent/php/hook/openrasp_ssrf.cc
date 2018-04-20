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
        zval *host = NULL;
        MAKE_STD_ZVAL(host);
        array_init(params);
        add_assoc_zval(params, "url", origin_url);
        Z_ADDREF_P(origin_url);
        php_url *url = php_url_parse_ex(Z_STRVAL_P(origin_url), Z_STRLEN_P(origin_url));
        if (url != NULL && url->host != NULL)
        {
            ZVAL_STRING(host, url->host, 1);
        }
        else
        {
            ZVAL_STRING(host, "", 1);
        }
        add_assoc_zval(params, "hostname", host);
        check("ssrf", params TSRMLS_CC);
    }
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