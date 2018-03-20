#include "openrasp_security_policy.h"
static void security_check(bool flag, int id, const char *msg TSRMLS_DC);
static bool strtobool(const char *str, int len);
#define SECURITY_CHECK(flag, id, msg) security_check(flag, id, msg TSRMLS_CC)
#define STRTOBOOL strtobool

PHP_MINIT_FUNCTION(openrasp_security_policy)
{
    SECURITY_CHECK(PG(allow_url_include) == 0, 4001, _("allow_url_include should be turned off to prevent File Inclusion vulnerability"));
    SECURITY_CHECK(PG(expose_php) == 0, 4002, _("expose_php should be turned off to prevent PHP Version information disclosure"));
    SECURITY_CHECK(PG(display_errors) == 0, 4003, _("display_errors should be turned off to prevent printing errors to the screen"));
    if (INI_STR("yaml.decode_php"))
    {
        SECURITY_CHECK(STRTOBOOL(INI_STR("yaml.decode_php"), strlen(INI_STR("yaml.decode_php"))) == false, 4004, _("yaml.decode_php should be turned off to prevent serializing php objects"));
    }
    return SUCCESS;
}

static void security_check(bool flag, int id, const char *msg TSRMLS_DC)
{
    if (!flag)
    {
        zval policy_id, message;
        INIT_ZVAL(policy_id);
        INIT_ZVAL(message);
        ZVAL_LONG(&policy_id, id);
        ZVAL_STRING(&message, msg, 0);

        zval result;
        INIT_ZVAL(result);
        ALLOC_HASHTABLE(Z_ARRVAL(result));
        zend_hash_init(Z_ARRVAL(result), 0, 0, 0, 0);
        Z_TYPE(result) = IS_ARRAY;
        add_assoc_zval(&result, "policy_id", &policy_id);
        add_assoc_zval(&result, "message", &message);
        policy_info(&result TSRMLS_CC);
        zval_dtor(&result);
    }
}

static bool strtobool(const char *str, int len)
{
    if (len == 2 && strcasecmp("on", str) == 0)
    {
        return true;
    }
    else if (len == 3 && strcasecmp("yes", str) == 0)
    {
        return true;
    }
    else if (len == 4 && strcasecmp("true", str) == 0)
    {
        return true;
    }
    else
    {
        return atoi(str);
    }
}