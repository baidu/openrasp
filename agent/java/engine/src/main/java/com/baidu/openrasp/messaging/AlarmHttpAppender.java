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

package com.baidu.openrasp.messaging;

import org.apache.log4j.AppenderSkeleton;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.spi.LoggingEvent;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by lxk on 9/12/17.
 */
public class AlarmHttpAppender extends AppenderSkeleton {
    private static final int DEFAULT_CONNECTION_TIMEOUT = 10000;
    private static final int DEFAULT_READ_TIMEOUT = 10000;
    private static final int DEFAULT_CACHE_FLUSH_TIME = 60000;

    private final HttpClient httpClient;
    private final EventCache eventCache;

    private String url;
    private int connectionTimeout = -1;
    private int readTimeout = -1;
    private int maxCacheSize = -1;
    private int cacheFlushTime = -1;

    public AlarmHttpAppender() {
        httpClient = new AsyncHttpClient();
        eventCache = new EventCache();
    }

    @Override
    public void close() {}

    @Override
    public boolean requiresLayout() {
        return false;
    }

    @Override
    public void activateOptions() {
        connectionTimeout = connectionTimeout < DEFAULT_CONNECTION_TIMEOUT ? DEFAULT_CONNECTION_TIMEOUT : connectionTimeout;
        readTimeout = readTimeout < DEFAULT_READ_TIMEOUT ? DEFAULT_READ_TIMEOUT : readTimeout;
        cacheFlushTime = cacheFlushTime < DEFAULT_CACHE_FLUSH_TIME ? DEFAULT_CACHE_FLUSH_TIME : cacheFlushTime;
        maxCacheSize = maxCacheSize < EventCache.DEFAULT_MAX_SIZE ? EventCache.DEFAULT_MAX_SIZE : maxCacheSize;
        eventCache.setSize(maxCacheSize);
        Timer timer = new Timer("async-http-appender-daemon",true);
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                sendEventsAsync();
            }
        }, cacheFlushTime, cacheFlushTime);

    }

    @Override
    protected void append(LoggingEvent event) {
        if (checkEntryConditions()) {
            subAppend(event);
        }
    }

    protected boolean checkEntryConditions() {
        if (this.closed) {
            LogLog.warn("Not allowed to write to a closed appender.");
            return false;
        } else if (httpClient == null) {
            LogLog.warn("HttpClient need to be initialized.");
            return false;
        } else if ((url == null) || url.trim().isEmpty()) {
            LogLog.warn("url need to be initialized.");
            return false;
        }
        return true;
    }

    protected void subAppend(LoggingEvent event) {
        if (eventCache.addEvent(event)
                || (httpClient instanceof AsyncHttpClient && ((AsyncHttpClient)httpClient).shouldSend())) {
            sendEventsAsync();
        }
    }

    protected synchronized void sendEventsAsync() {
        if (!eventCache.isEmpty()) {
            String attackInfos = eventCache.getJsonBody();
            eventCache.clear();
            httpClient.request(url, attackInfos, connectionTimeout, readTimeout);
        }
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public int getConnectionTimeout() {
        return connectionTimeout;
    }

    public void setConnectionTimeout(int connectionTimeout) {
        this.connectionTimeout = connectionTimeout;
    }

    public int getReadTimeout() {
        return readTimeout;
    }

    public void setReadTimeout(int readTimeout) {
        this.readTimeout = readTimeout;
    }

    public int getMaxCacheSize() {
        return maxCacheSize;
    }

    public void setMaxCacheSize(int maxCacheSize) {
        this.maxCacheSize = maxCacheSize;
    }

    public int getCacheFlushTime() {
        return cacheFlushTime;
    }

    public void setCacheFlushTime(int cacheFlushTime) {
        this.cacheFlushTime = cacheFlushTime;
    }

}
