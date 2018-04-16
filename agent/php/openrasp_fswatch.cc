#include "openrasp_fswatch.h"
#include "openrasp_ini.h"
#include "libfswatch/c++/monitor.hpp"
#include <thread>
#include <signal.h>
#include <algorithm>
#include <exception>
#ifndef PHP_WIN32
static std::vector<std::pair<std::string, int>> supported_sapis{
    {"apache2handler", SIGUSR1},
    {"fpm-fcgi", SIGUSR2},
};
static std::vector<std::pair<std::string, int>>::iterator supported_sapi;
#else
#include <windows.h>
void (*ap_signal_parent)(int) = nullptr;
#endif
static fsw::monitor *monitor = nullptr;
static std::thread *fswatch_thread = nullptr;
PHP_MINIT_FUNCTION(openrasp_fswatch)
{
#ifndef PHP_WIN32
    supported_sapi = std::find_if(supported_sapis.begin(), supported_sapis.end(),
                                  [](std::pair<std::string, int> i) { return i.first.compare(sapi_module.name) == 0; });
    if (supported_sapi == supported_sapis.end())
    {
        //openrasp_error(E_WARNING, CONFIG_ERROR, _("SAPI %s does not support automatic reloading, file monitor is deactivated"), sapi_module.name);
        return SUCCESS;
    }
#else
    if (getenv("AP_PARENT_PID") != nullptr ||
        (*(void **)(&ap_signal_parent) = GetProcAddress(GetModuleHandle("libhttpd.dll"), "ap_signal_parent")) == nullptr)
    {
        return SUCCESS;
    }
#endif
    try
    {
        std::vector<std::string> paths;
        paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("plugins"));
        paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("assets"));

        monitor = fsw::monitor_factory::create_monitor(
            fsw_monitor_type::system_default_monitor_type, paths,
            [](const std::vector<fsw::event> &events, void *ctx) {
#ifndef PHP_WIN32
                if (raise(supported_sapi->second))
                {
                    openrasp_error(E_WARNING, CONFIG_ERROR, _("Failed to reload %s"), sapi_module.name);
                }
#else
                ap_signal_parent(2);
#endif
                monitor->stop();
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

        fswatch_thread = new std::thread([]() {
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
        });
        fswatch_thread->detach();
    }
    catch (std::exception &conf)
    {
        openrasp_error(E_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: %s"), conf.what());
    }
    catch (...)
    {
        openrasp_error(E_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: unknown error."));
    }

    return SUCCESS;
}
PHP_MSHUTDOWN_FUNCTION(openrasp_fswatch)
{
    delete monitor;
    monitor = nullptr;
    delete fswatch_thread;
    fswatch_thread = nullptr;
    return SUCCESS;
}