#pragma once

#include "openrasp_hook.h"

OPENRASP_HOOK_FUNCTION(curl_exec)
{
    zval **zid, function_name, opt, origin_url, effective_url, *args[2];
    INIT_ZVAL(function_name);
    INIT_ZVAL(opt);
    INIT_ZVAL(origin_url);
    INIT_ZVAL(effective_url);
    ZVAL_STRING(&function_name, "curl_getinfo", 0); // 不需要 zval_dtor

    int argc = MIN(1, ZEND_NUM_ARGS());
    if (openrasp_check_type_ignored(ZEND_STRL("ssrf") TSRMLS_CC) ||
        argc <= 0 ||
        zend_get_parameters_ex(argc, &zid) != SUCCESS ||
        Z_TYPE_PP(zid) != IS_RESOURCE)
    {
        goto PASSTHRU;
    }
    if (!zend_get_constant(ZEND_STRL("CURLINFO_EFFECTIVE_URL"), &opt TSRMLS_CC))
    {
        goto PASSTHRU;
    }
    args[0] = *zid;
    args[1] = &opt;
    if (call_user_function(EG(function_table), NULL, &function_name, &origin_url, 2, args TSRMLS_CC) != SUCCESS ||
        Z_TYPE(origin_url) != IS_STRING)
    {
        goto PASSTHRU;
    }
    {
        zval *params;
        MAKE_STD_ZVAL(params);
        zval *host = NULL;
        MAKE_STD_ZVAL(host);
        array_init(params);
        add_assoc_zval(params, "url", &origin_url);
        Z_ADDREF(origin_url);
        php_url *url = php_url_parse_ex(Z_STRVAL(origin_url), Z_STRLEN(origin_url));
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
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    if (call_user_function(EG(function_table), NULL, &function_name, &effective_url, 2, args TSRMLS_CC) != SUCCESS ||
        Z_TYPE(effective_url) != IS_STRING)
    {
        goto PASSTHRU;
    }
    if ((strncasecmp(Z_STRVAL(effective_url), "file", 4) == 0 || strncasecmp(Z_STRVAL(effective_url), "scp", 3) == 0) &&
        strcmp(Z_STRVAL(effective_url), Z_STRVAL(origin_url)) != 0)
    {
        zval *attack_params = NULL;
        MAKE_STD_ZVAL(attack_params);
        ZVAL_STRING(attack_params, Z_STRVAL(effective_url), 1);
        zval *plugin_message = NULL;
        MAKE_STD_ZVAL(plugin_message);
        char *message_str = NULL;
        spprintf(&message_str, 0, _("ssrf found : %s"), Z_STRVAL(effective_url));
        ZVAL_STRING(plugin_message, message_str, 1);
        efree(message_str);
        openrasp_buildin_php_risk_handle(1, "ssrf", 100, attack_params, plugin_message TSRMLS_CC);       
    }
    zval_dtor(&opt);
    zval_dtor(&origin_url);
    zval_dtor(&effective_url);
    return;
PASSTHRU:
    zval_dtor(&opt);
    zval_dtor(&origin_url);
    zval_dtor(&effective_url);
    origin_function(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}