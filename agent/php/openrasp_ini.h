#pragma once

#include "openrasp.h"
#include <string>
#include <unordered_set>

ZEND_INI_MH(OnUpdateOpenraspIntGEZero);
ZEND_INI_MH(OnUpdateOpenraspCString);
ZEND_INI_MH(OnUpdateOpenraspBool);
ZEND_INI_MH(OnUpdateOpenraspSet);

class Openrasp_ini
{
  public:
    char *root_dir;
    char *locale;
    unsigned int timeout_ms = 100;
    char *syslog_server_address;
    int log_maxburst = 1000;
    int syslog_facility;
    bool syslog_alarm_enable = 0;
    char *inject_html_urlprefix;
    unsigned int slowquery_min_rows = 500;
    bool enforce_policy = false;
    char *block_url;
    std::unordered_set<std::string> hooks_ignore;
    std::unordered_set<std::string> callable_blacklists;
};

extern Openrasp_ini openrasp_ini;
