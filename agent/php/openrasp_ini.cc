#include "openrasp_ini.h"
#include <regex>
#include <limits>

Openrasp_ini openrasp_ini;

ZEND_INI_MH(OnUpdateOpenraspIntGEZero)
{
    long tmp = zend_atol(new_value, new_value_length);
    if (tmp < 0 || tmp > std::numeric_limits<unsigned int>::max())
    {
        return FAILURE;
    }
    *reinterpret_cast<int *>(mh_arg1) = tmp;
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspCString)
{
    *reinterpret_cast<char **>(mh_arg1) = new_value_length ? new_value : nullptr;
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspBool)
{
    bool *tmp = reinterpret_cast<bool *>(mh_arg1);
    if (new_value_length == 2 && strcasecmp("on", new_value) == 0)
    {
        *tmp = true;
    }
    else if (new_value_length == 3 && strcasecmp("yes", new_value) == 0)
    {
        *tmp = true;
    }
    else if (new_value_length == 4 && strcasecmp("true", new_value) == 0)
    {
        *tmp = true;
    }
    else
    {
        *tmp = atoi(new_value);
    }
    return SUCCESS;
}

ZEND_INI_MH(OnUpdateOpenraspSet)
{
    std::unordered_set<std::string> *p = reinterpret_cast<std::unordered_set<std::string> *>(mh_arg1);
    p->clear();
    if (new_value)
    {
        std::regex re(R"([\s,]+)");
        const std::cregex_token_iterator end;
        for (std::cregex_token_iterator it(new_value, new_value + new_value_length, re, -1); it != end; it++)
        {
            p->insert(it->str());
        }
    }
    return SUCCESS;
}