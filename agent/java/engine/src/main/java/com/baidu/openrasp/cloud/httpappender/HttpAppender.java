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

package com.baidu.openrasp.cloud.httpappender;

import com.baidu.openrasp.cloud.CloudHttp;
import com.baidu.openrasp.cloud.ThreadPool;
import com.baidu.openrasp.cloud.model.AppenderCache;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.messaging.ExceptionModel;
import com.baidu.openrasp.plugin.info.ExceptInfo;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.Level;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.spi.LoggingEvent;
import org.apache.log4j.spi.ThrowableInformation;

import java.lang.management.ManagementFactory;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * @description: 日志上传appender
 * @author: anyang
 * @create: 2018/09/20 09:53
 */
public class HttpAppender extends AppenderSkeleton {
    private ThreadPool threadPool;

    public HttpAppender() {
        this.threadPool = new ThreadPool();
    }

    private boolean checkEntryConditions() {
        if (threadPool == null) {
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
            String logger = getLogger(loggingEvent.getLoggerName());
            JsonElement jsonElement = null;
            if ("root".equals(logger)) {
                if ((loggingEvent.getLevel().equals(Level.WARN) || loggingEvent.getLevel().equals(Level.ERROR))
                        && loggingEvent.getMessage() instanceof ExceptionModel) {
                    jsonElement = new JsonParser().parse(generateJson(loggingEvent));
                }
            } else {
                jsonElement = new JsonParser().parse(loggingEvent.getRenderedMessage());
            }
            if (jsonElement != null) {
                JsonArray jsonArray = mergeFromAppenderCache(loggingEvent.getLoggerName(), jsonElement);
                if (logger != null) {
                    String requestUrl = getUrl(logger);
                    if (requestUrl != null) {
                        Runnable runnable = createTask(requestUrl, new Gson().toJson(jsonArray), logger, jsonElement);
                        threadPool.getThreadPool().execute(runnable);
                    }
                }
            }
        }
    }

    private Runnable createTask(final String url, final String content, final String loggerName, final JsonElement currentLog) {
        return new Runnable() {
            @Override
            public void run() {
                GenericResponse response = new CloudHttp().logRequest(url, content);
                if (response != null) {
                    Integer responseCode = response.getResponseCode();
                    if (responseCode != null && responseCode >= 200 && responseCode < 300) {
                        return;
                    }
                }
                AppenderCache.setCache(loggerName, currentLog);
            }
        };
    }

    private JsonArray mergeFromAppenderCache(String loggerName, JsonElement currnetLog) {
        Set<JsonElement> sets = new HashSet<JsonElement>();
        sets.add(currnetLog);
        ConcurrentLinkedQueue<JsonElement> queue = AppenderCache.getCache(getLogger(loggerName));
        if (queue != null && !queue.isEmpty()) {
            sets.addAll(queue);
        }
        JsonArray jsonArray = new JsonArray();
        for (JsonElement element : sets) {
            jsonArray.add(element);
        }
        return jsonArray;
    }

    private String getLogger(String loggerName) {
        String name;
        if (loggerName.contains("policy_alarm")) {

            name = "policy_alarm";

        } else if (loggerName.contains("alarm")) {

            name = "alarm";

        } else if (loggerName.contains("js")) {

            name = "plugin";

        } else {

            name = "root";
        }
        return name;
    }

    private String getUrl(String logger) {
        String url = null;
        if ("policy_alarm".equals(logger)) {
            url = CloudRequestUrl.CLOUD_POLICY_ALARM_HTTP_APPENDER_URL;
        } else if ("alarm".equals(logger)) {
            url = CloudRequestUrl.CLOUD_ALARM_HTTP_APPENDER_URL;
        } else if ("plugin".equals(logger)) {
            url = CloudRequestUrl.CLOUD_PLUGIN_HTTP_APPENDER_URL;
        } else {
            url = CloudRequestUrl.CLOUD_EXCEPTION_HTTP_APPENDER_URL;
        }
        return url;
    }

    /**
     * 处理log4j的ROOT logger上传日志的loggingEvent
     */
    private String generateJson(LoggingEvent loggingEvent) {
        String level = loggingEvent.getLevel().toString();
        ExceptionModel model = (ExceptionModel) loggingEvent.getMessage();
        String message = model.getMessage();
        int errorCode = model.getErrorType().getCode();
        ThrowableInformation information = loggingEvent.getThrowableInformation();
        Throwable t = information != null ? information.getThrowable() : null;
        StackTraceElement[] traceElements = t != null ? t.getStackTrace() : new StackTraceElement[]{};
        ExceptInfo info = new ExceptInfo(level, message, errorCode, getProcessID(), traceElements);
        return new Gson().toJson(info.getInfo());
    }

    private int getProcessID() {
        try {
            String[] pids = ManagementFactory.getRuntimeMXBean().getName().split("@");
            return Integer.parseInt(pids[0]);
        } catch (Throwable e) {
            return -1;
        }
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
