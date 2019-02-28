/*
 * Copyright 2017-2019 Baidu Inc.
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

package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.config.Config;

public interface CloudRequestUrl {
    String cloudAddress = Config.getConfig().getCloudAddress();
    String CLOUD_HEART_BEAT_URL = cloudAddress + "/v1/agent/heartbeat";
    String CLOUD_REGISTER_URL = cloudAddress + "/v1/agent/rasp";
    String CLOUD_ALARM_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/attack";
    String CLOUD_POLICY_ALARM_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/policy";
    String CLOUD_PLUGIN_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/plugin";
    String CLOUD_EXCEPTION_HTTP_APPENDER_URL = cloudAddress + "/v1/agent/log/error";
    String CLOUD_STATISTICS_REPORT_URL = cloudAddress + "/v1/agent/report";
}
