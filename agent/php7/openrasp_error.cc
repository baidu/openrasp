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

#include "utils/json_reader.h"
#include "openrasp.h"
#include "openrasp_ini.h"
#include "openrasp_log.h"
#include "utils/time.h"
#include "utils/hostname.h"
#include "agent/shared_config_manager.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif

using openrasp::JsonReader;
using openrasp::SharedConfigManager;

void openrasp_error(int type, openrasp_error_code code, const char *format, ...)
{
    va_list arg;
    char *message = nullptr;
    va_start(arg, format);
    vspprintf(&message, 0, format, arg);
    va_end(arg);
    if (log_module_initialized())
    {
        JsonReader json_reader;
        json_reader.write_string({"level"}, RaspLoggerEntry::level_to_name((severity_level)type));
        json_reader.write_int64({"error_code"}, (int64_t)code);
        if (openrasp_ini.app_id)
        {
            json_reader.write_string({"app_id"}, openrasp_ini.app_id);
        }
        json_reader.write_string({"rasp_id"}, openrasp::scm->get_rasp_id());
        std::string log_time = openrasp::format_time(RaspLoggerEntry::rasp_rfc3339_format,
                                                     strlen(RaspLoggerEntry::rasp_rfc3339_format), (long)time(NULL));
        json_reader.write_string({"event_time"}, log_time);
        json_reader.write_string({"server_hostname"}, openrasp::get_hostname());
        json_reader.write_map_to_array({"server_nic"}, "name", "ip", get_if_addr_map());
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
        if (openrasp::oam != nullptr)
        {
            json_reader.write_int64({"pid"}, openrasp::oam->get_master_pid());
        }
#endif
        json_reader.write_string({"message"}, message);
        std::string error_content = json_reader.dump();
        TSRMLS_FETCH();
        LOG_G(rasp_logger).log((severity_level)type, error_content.c_str(), error_content.length(), true, false);
    }
    else
    {
        //always convert to E_WARNING level
        zend_error(E_WARNING, "[OpenRASP] %d %s", code, message);
    }
    efree(message);
}
