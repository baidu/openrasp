/*
 * Copyright 2017-2021 Baidu Inc.
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
static pid_t master_pid = 0;
PHP_MINIT_FUNCTION(openrasp_fswatch)
{
    if (master_pid != 0)
    {
        return SUCCESS;
    }
#ifndef PHP_WIN32
    supported_sapi = std::find_if(supported_sapis.begin(), supported_sapis.end(),
                                  [](std::pair<std::string, int> i) { return i.first.compare(sapi_module.name) == 0; });
    if (supported_sapi == supported_sapis.end())
    {
        return SUCCESS;
    }
#else
    if (getenv("AP_PARENT_PID") != nullptr)
    {
        master_pid = std::stol(getenv("AP_PARENT_PID"));
        return SUCCESS;
    }
    HMODULE module = GetModuleHandle("libhttpd.dll");
    *(void **)(&ap_signal_parent) = GetProcAddress(module, "ap_signal_parent");
    if (ap_signal_parent == nullptr)
    {
        *(void **)(&ap_signal_parent) = GetProcAddress(module, "_ap_signal_parent@4");
    }
    if (ap_signal_parent == nullptr)
    {
        return SUCCESS;
    }
#endif
    try
    {
        std::vector<std::string> paths;
        paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("plugins"));
        paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("assets"));
        paths.push_back(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("conf"));

        monitor = fsw::monitor_factory::create_monitor(
            fsw_monitor_type::system_default_monitor_type, paths,
            [](const std::vector<fsw::event> &events, void *ctx) {
                try
                {
#ifndef PHP_WIN32
                    if (raise(supported_sapi->second))
                    {
                        openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("Failed to reload %s"), sapi_module.name);
                    }
#else
                    ap_signal_parent(2);
#endif
                }
                catch (...)
                {
                    openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("An exception occurred while reloading master process, but this message may not be visible"));
                }
            });

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
        path_filters.push_back(fsw::monitor_filter{R"([/\\]conf([/\\]openrasp\.yml)?$)", fsw_filter_type::filter_include, false, false});
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
                openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("Fswatch error: %s"), conf.what());
            }
            catch (...)
            {
                openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("Fswatch error: unknown error"));
            }
            delete monitor;
            monitor = nullptr;
        });
    }
    catch (std::exception &conf)
    {
        openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: %s"), conf.what());
    }
    catch (...)
    {
        openrasp_error(LEVEL_WARNING, FSWATCH_ERROR, _("Failed to initialize fswatch: unknown error."));
    }

    master_pid = getpid();
    return SUCCESS;
}
PHP_MSHUTDOWN_FUNCTION(openrasp_fswatch)
{
    if (master_pid != getpid())
    {
        return SUCCESS;
    }
    if (monitor)
    {
        monitor->stop();
    }
    if (fswatch_thread && fswatch_thread->joinable())
    {
        fswatch_thread->join();
    }
    delete fswatch_thread;
    fswatch_thread = nullptr;
    master_pid = 0;
    return SUCCESS;
}