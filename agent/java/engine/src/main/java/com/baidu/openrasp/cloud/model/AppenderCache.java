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

import com.google.gson.JsonElement;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * @description: 缓存需要重传的日志
 * @author: anyang
 * @create: 2018/09/20 13:59
 */
public class AppenderCache {
    private static ConcurrentHashMap<String, ConcurrentLinkedQueue<JsonElement>> appenderCache = new ConcurrentHashMap<String, ConcurrentLinkedQueue<JsonElement>>();
    private static final int APPENDER_LRUCACHE_SIZE = 500;

    public static void setCache(String key, JsonElement value) {
        if (appenderCache.containsKey(key)) {
            ConcurrentLinkedQueue<JsonElement> temp = appenderCache.get(key);
            if (temp.size() < APPENDER_LRUCACHE_SIZE) {
                temp.add(value);
            } else {
                temp.poll();
                temp.add(value);
            }
            appenderCache.put(key, temp);
        } else {
            ConcurrentLinkedQueue<JsonElement> cache = new ConcurrentLinkedQueue<JsonElement>();
            cache.add(value);
            appenderCache.put(key, cache);
        }
    }

    public static ConcurrentLinkedQueue<JsonElement> getCache(String key) {
        if (appenderCache.containsKey(key)) {
            return appenderCache.get(key);
        }
        return null;
    }

}
