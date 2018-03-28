#include "openrasp_fswatch.h"
#include "openrasp_ini.h"
#include "libfswatch/c++/monitor.hpp"
#include <thread>
#include <signal.h>
#include <algorithm>
#include <exception>
static std::vector<std::pair<std::string, int>> supported_sapis{
    {"apache2handler", SIGUSR1},
    {"fpm-fcgi", SIGUSR2},
};
static std::vector<std::pair<std::string, int>>::iterator supported_sapi;
static std::thread *fswatch_thread = nullptr;
PHP_MINIT_FUNCTION(openrasp_fswatch)
{
    supported_sapi = std::find_if(supported_sapis.begin(), supported_sapis.end(),
                                  [](std::pair<std::string, int> i) { return i.first.compare(sapi_module.name) == 0; });
    if (supported_sapi == supported_sapis.end())
    {
        //openrasp_error(E_WARNING, CONFIG_ERROR, _("SAPI %s does not support automatic reloading, file monitor is deactivated"), sapi_module.name);
        return SUCCESS;
    }
    std::vector<std::string> paths;
    paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("plugins"));
    paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("assets"));
    fsw::monitor *monitor = nullptr;
    try
    {
        monitor = fsw::monitor_factory::create_monitor(
            fsw_monitor_type::system_default_monitor_type, paths,
            [](const std::vector<fsw::event> &events, void *ctx) {
                if (raise(supported_sapi->second))
                {
                    openrasp_error(E_WARNING, CONFIG_ERROR, _("Failed to reload %s"), sapi_module.name);
                }
            } TSRMLS_CC);

        std::vector<fsw_event_type_filter> event_filters;
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::IsFile});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::Created});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::MovedFrom});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::MovedTo});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::Removed});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::Renamed});
        event_filters.push_back(fsw_event_type_filter{fsw_event_flag::Updated});
        monitor->set_event_type_filters(event_filters);

        std::vector<fsw::monitor_filter> path_filters;
        path_filters.push_back(fsw::monitor_filter{".*", fsw_filter_type::filter_exclude, false, false});
        path_filters.push_back(fsw::monitor_filter{R"([/\\]plugins([/\\][^/\\]+\.js)?$)", fsw_filter_type::filter_include, false, false});
        path_filters.push_back(fsw::monitor_filter{R"([/\\]assets([/\\]inject\.html)?$)", fsw_filter_type::filter_include, false, false});
        monitor->set_filters(path_filters);

        monitor->set_recursive(false);
        monitor->set_latency(1);
    }
    catch (std::exception &conf)
    {
        openrasp_error(E_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: %s"), conf.what());
    }
    catch (...)
    {
        openrasp_error(E_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: unknown error."));
    }
    fswatch_thread = new std::thread([monitor]() {
        try
        {
            monitor->start();
        }
        catch (std::exception &conf)
        {
            openrasp_error(E_WARNING, FSWATCH_ERROR, _("Fswatch error: %s"), conf.what());
        }
        catch (...)
        {
            openrasp_error(E_WARNING, FSWATCH_ERROR, _("Fswatch error: unknown error"));
        }
        monitor->stop();
        delete monitor;
    });
    fswatch_thread->detach();
    return SUCCESS;
}
PHP_MSHUTDOWN_FUNCTION(openrasp_fswatch)
{
    delete fswatch_thread;
    fswatch_thread = nullptr;
    return SUCCESS;
}