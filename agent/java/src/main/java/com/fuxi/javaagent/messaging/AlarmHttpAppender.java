/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.messaging;

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
