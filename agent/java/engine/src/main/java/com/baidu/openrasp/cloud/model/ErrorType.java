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

public enum ErrorType {
    FSWATCH_ERROR(20001, "File Watch Error"),
    RUNTIME_ERROR(20002, "Common Error"),
    CONFIG_ERROR(20004, "Config Error"),
    PLUGIN_ERROR(20005, "Plugin Error"),
    REQUEST_ERROR(20006, "Request Error"),
    HOOK_ERROR(20007, "Hook Error"),
    REGISTER_ERROR(20008, "Cloud Control Registered Failed"),
    HEARTBEAT_ERROR(20009, "Cloud Control Send HeartBeat Failed"),
    STATISTICSREPORT_ERROR(20010, "Cloud Control Statistics,Report Failed"),
    HTTPAPPENDER_ERROR(20011, "Cloud Control Log Upload Failed"),
    DETECT_SERVER_ERROR(20012, "detect server Error"),
    REFLECTION_ERROR(20013, "Reflex Failed");
    private int code;
    private String message;

    ErrorType(int code, String message) {
        this.code = code;
        this.message = message;
    }

    public int getCode() {
        return code;
    }

    public String getMessage() {
        return message;
    }

    @Override
    public String toString() {
        return code + ":" + message;
    }
}
