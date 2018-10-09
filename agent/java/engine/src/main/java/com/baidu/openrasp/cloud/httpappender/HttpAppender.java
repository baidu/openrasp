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

package com.baidu.openrasp.cloud.httpappender;

import com.baidu.openrasp.cloud.CloudHttp;
import com.baidu.openrasp.cloud.CloudHttpPool;
import com.baidu.openrasp.cloud.model.AppenderCache;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonParser;
import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.spi.LoggingEvent;

import java.util.HashSet;
import java.util.Set;

/**
 * @description: 日志上传appender
 * @author: anyang
 * @create: 2018/09/20 09:53
 */
public class HttpAppender extends AppenderSkeleton {
    private CloudHttp cloudHttp;

    public HttpAppender() {
        this.cloudHttp = new CloudHttpPool();
    }

    private boolean checkEntryConditions() {
        if (cloudHttp == null) {
            LogLog.warn("Http need to be initialized.");
            return false;

        } else if (this.closed) {
            LogLog.warn("Not allowed to write to a closed appender.");
            return false;

        }
        return true;
    }

    @Override
    protected void append(LoggingEvent loggingEvent) {
        if (checkEntryConditions()) {
            String jsonString = loggingEvent.getRenderedMessage();
            JsonArray jsonArray = mergeFromAppenderCache(loggingEvent.getLoggerName(), jsonString);
            String logger = getLogger(loggingEvent.getLoggerName());
            GenericResponse response = cloudHttp.request(getUrl(logger), new Gson().toJson(jsonArray));
            if (response != null) {
                Integer responseCode = response.getResponseCode();
                if (responseCode != null && responseCode >= 200 && responseCode < 300) {
                    return;
                }
            }
            AppenderCache.setCache(logger, jsonString);
        }
    }

    private JsonArray mergeFromAppenderCache(String loggerName, String currnetLog) {
        Set<String> sets = new HashSet<String>();
        sets.add(currnetLog);
        Set<String> set = AppenderCache.getCache(getLogger(loggerName));
        if (set != null && !set.isEmpty()) {
            sets.addAll(set);
        }
        String json = new Gson().toJson(sets);
        return new JsonParser().parse(json).getAsJsonArray();
    }

    private String getLogger(String loggerName) {
        String name;
        if (loggerName.contains("policy_alarm")) {
            name = "policy_alarm";
        } else if (loggerName.contains("alarm")) {
            name = "alarm";
        } else {
            name = "plugin";
        }
        return name;
    }

    private String getUrl(String logger) {
        String url;
        if ("policy_alarm".equals(logger)) {
            url = CloudRequestUrl.CLOUD_POLICY_ALARM_HTTP_APPENDER_URL;
        } else if ("alarm".equals(logger)) {
            url = CloudRequestUrl.CLOUD_ALARM_HTTP_APPENDER_URL;
        } else {
            url = CloudRequestUrl.CLOUD_PLUGIN_HTTP_APPENDER_URL;
        }
        return url;
    }

    @Override
    public void close() {

    }

    @Override
    public boolean requiresLayout() {
        return false;
    }

    @Override
    public void activateOptions() {
    }
}
