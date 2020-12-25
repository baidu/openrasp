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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}
#include <sstream>
#include <fstream>
#include "openrasp_v8.h"
#include "openrasp_hook.h"
#include "openrasp_ini.h"
#include "openrasp_output_detect.h"
#include "agent/shared_config_manager.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

namespace openrasp
{
openrasp_v8_process_globals process_globals;
} // namespace openrasp

using namespace openrasp;

ZEND_DECLARE_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8)
{
#ifdef ZTS
    new (openrasp_v8_globals) _zend_openrasp_v8_globals;
#endif
}

PHP_GSHUTDOWN_FUNCTION(openrasp_v8)
{
    if (openrasp_v8_globals->isolate)
    {
        Platform::Get()->Startup();
        openrasp_v8_globals->isolate->Dispose();
        openrasp_v8_globals->isolate = nullptr;
    }
#ifdef ZTS
    openrasp_v8_globals->~_zend_openrasp_v8_globals();
#endif
}

PHP_MINIT_FUNCTION(openrasp_v8)
{
    ZEND_INIT_MODULE_GLOBALS(openrasp_v8, PHP_GINIT(openrasp_v8), PHP_GSHUTDOWN(openrasp_v8));

    // initializes v8 only once
    std::call_once(process_globals.init_v8_once, []() {
        Initialize(1, plugin_log);
    });

#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp_ini.remote_management_enable && oam != nullptr)
    {
        Platform::Get()->Shutdown();
        return SUCCESS;
    }
#endif

    load_plugins();

    if (!process_globals.snapshot_blob)
    {
        Platform::Get()->Startup();
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Snapshot *snapshot = new Snapshot(process_globals.plugin_config, process_globals.plugin_src_list, OpenRASPInfo::PHP_OPENRASP_VERSION, millis, nullptr);
        if (!snapshot->IsOk())
        {
            delete snapshot;
            openrasp_error(LEVEL_WARNING, PLUGIN_ERROR, _("Fail to initialize builtin js code."));
        }
        else
        {
            process_globals.snapshot_blob = snapshot;
            std::map<OpenRASPCheckType, OpenRASPActionType> type_action_map;
            std::map<std::string, std::string> buildin_action_map = CheckTypeTransfer::instance().get_buildin_action_map();
            auto duration = std::chrono::system_clock::now().time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            Isolate *isolate = Isolate::New(snapshot, millis);
            extract_buildin_action(isolate, buildin_action_map);
            for (auto iter = buildin_action_map.begin(); iter != buildin_action_map.end(); iter++)
            {
                type_action_map.insert({CheckTypeTransfer::instance().name_to_type(iter->first), string_to_action(iter->second)});
            }
            openrasp::scm->set_buildin_check_action(type_action_map);
            openrasp::scm->set_mysql_error_codes(extract_int64_array(isolate, "RASP.algorithmConfig.sql_exception.mysql.error_code", SharedConfigBlock::MYSQL_ERROR_CODE_MAX_SIZE));
            openrasp::scm->set_sqlite_error_codes(extract_int64_array(isolate, "RASP.algorithmConfig.sql_exception.sqlite.error_code", SharedConfigBlock::SQLITE_ERROR_CODE_MAX_SIZE));
            openrasp::scm->build_pg_error_array(isolate);
            openrasp::scm->build_env_key_array(isolate);
            isolate->Dispose();
        }
        Platform::Get()->Shutdown();
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(openrasp_v8)
{
    ZEND_SHUTDOWN_MODULE_GLOBALS(openrasp_v8, PHP_GSHUTDOWN(openrasp_v8));

    // Disposing v8 is permanent, it cannot be reinitialized,
    // it should generally not be necessary to dispose v8 before exiting a process,
    // so skip this step for module graceful reload
    // v8::V8::Dispose();
    // Platform::Get()->Shutdown();
    delete process_globals.snapshot_blob;
    process_globals.snapshot_blob = nullptr;

    return SUCCESS;
}

PHP_RINIT_FUNCTION(openrasp_v8)
{
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp_ini.remote_management_enable && oam != nullptr)
    {
        uint64_t timestamp = oam->get_plugin_update_timestamp();
        if (timestamp > 0 &&
            (!process_globals.snapshot_blob ||
             process_globals.snapshot_blob->IsExpired(timestamp)))
        {
            std::unique_lock<std::mutex> lock(process_globals.mtx, std::try_to_lock);
            if (lock &&
                (!process_globals.snapshot_blob ||
                 process_globals.snapshot_blob->IsExpired(timestamp)))
            {
                std::string filename = std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("snapshot.dat");
                Snapshot *blob = new Snapshot(filename, timestamp);
                if (!blob->IsOk())
                {
                    delete blob;
                }
                else
                {
                    delete process_globals.snapshot_blob;
                    process_globals.snapshot_blob = blob;
                    OPENRASP_HOOK_G(lru).clear();
                }
            }
        }
    }
#endif
    if (process_globals.snapshot_blob)
    {
        if (!OPENRASP_V8_G(isolate) || OPENRASP_V8_G(isolate)->IsExpired(process_globals.snapshot_blob->timestamp))
        {
            std::unique_lock<std::mutex> lock(process_globals.mtx, std::try_to_lock);
            if (lock)
            {
                Platform::Get()->Startup();
                if (OPENRASP_V8_G(isolate))
                {
                    OPENRASP_V8_G(isolate)->Dispose();
                }
                auto isolate = Isolate::New(process_globals.snapshot_blob, process_globals.snapshot_blob->timestamp);
                v8::HandleScope handle_scope(isolate);
                isolate->GetData()->request_context_templ.Reset(isolate, CreateRequestContextTemplate(isolate));
                OPENRASP_V8_G(isolate) = isolate;
                {
                    static const std::vector<std::string> default_callable_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};
                    static const std::string default_echo_filter_regex = "<![\\\\-\\\\[A-Za-z]|<([A-Za-z]{1,12})[\\\\/ >]";
                    static const std::string default_filter_regex = "<![\\\\-\\\\[A-Za-z]|<([A-Za-z]{1,12})[\\\\/ >]";
                    static const int64_t default_min_param_length = 15;
                    static const int64_t default_max_detection_num = 10;

                    std::vector<std::string> callable_blacklist_vector = extract_string_array(isolate, "RASP.algorithmConfig.webshell_callable.functions", 100, default_callable_blacklist);
                    OPENRASP_HOOK_G(callable_blacklist) = std::unordered_set<std::string>(callable_blacklist_vector.begin(), callable_blacklist_vector.end());
                    OPENRASP_HOOK_G(echo_filter_regex) = extract_string(isolate, "RASP.algorithmConfig.xss_echo.filter_regex", default_echo_filter_regex);
                    OUTPUT_G(filter_regex) = extract_string(isolate, "RASP.algorithmConfig.xss_userinput.filter_regex", default_filter_regex);
                    OUTPUT_G(min_param_length) = extract_int64(isolate, "RASP.algorithmConfig.xss_userinput.min_length", default_min_param_length);
                    OUTPUT_G(max_detection_num) = extract_int64(isolate, "RASP.algorithmConfig.xss_userinput.max_detection_num", default_max_detection_num);
                }
            }
        }
    }
    if (OPENRASP_V8_G(isolate))
    {
        OPENRASP_V8_G(isolate)->GetData()->request_context.Reset();
    }
    return SUCCESS;
}
