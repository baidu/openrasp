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

package com.baidu.openrasp.plugin.info;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.OSUtil;
import com.google.gson.Gson;

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

/**
 * @description: 异常信息类，用于异常上报
 * @author: anyang
 * @create: 2018/12/17 14:52
 */
public class ExceptInfo {
    private String appId;
    private String level;
    private String message;
    private Long createTime;
    private int pid;
    private int errorCode;
    private String stackTrace;

    public ExceptInfo(String level, String message, int errorCode, int pid, StackTraceElement[] trace) {
        this(Config.getConfig().getCloudAppId(), level, message, errorCode, System.currentTimeMillis(), pid, trace);
    }

    public ExceptInfo(String appId, String level, String message, int errorCode, Long createTime, int pid
            , StackTraceElement[] trace) {
        this.appId = appId;
        this.level = level;
        this.message = message;
        this.errorCode = errorCode;
        this.createTime = createTime;
        this.pid = pid;
        this.stackTrace = stringify(trace);
    }

    public Map<String, Object> getInfo() {
        Map<String, Object> info = new HashMap<String, Object>();
        Timestamp createTime = new Timestamp(this.createTime);

        info.put("pid", this.pid);
        info.put("event_time", new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ").format(createTime));
        info.put("rasp_id", CloudCacheModel.getInstance().getRaspId());
        info.put("app_id", this.appId);
        info.put("level", this.level);
        info.put("error_code", this.errorCode);
        info.put("message", this.message);
        info.put("stack_trace", this.stackTrace);
        info.put("server_hostname", OSUtil.getHostName());
        info.put("server_nic", OSUtil.getIpAddress());
        return info;
    }

    public String getAppId() {
        return appId;
    }

    public void setAppId(String appId) {
        this.appId = appId;
    }

    public String getLevel() {
        return level;
    }

    public void setLevel(String level) {
        this.level = level;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Long getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Long createTime) {
        this.createTime = createTime;
    }

    public int getPid() {
        return pid;
    }

    public void setPid(int pid) {
        this.pid = pid;
    }

    private String stringify(StackTraceElement[] trace) {
        StringBuilder ret = new StringBuilder();
        for (StackTraceElement element : trace) {
            ret.append(element);
            ret.append("\n");
        }
        return ret.toString();
    }

    @Override
    public String toString() {
        Map<String, Object> info = getInfo();
        return new Gson().toJson(info);
    }
}
