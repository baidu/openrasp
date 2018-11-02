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

package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.tool.LRUCache;

import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @description: 缓存需要重传的日志
 * @author: anyang
 * @create: 2018/09/20 13:59
 */
public class AppenderCache {
    private static ConcurrentHashMap<String, LRUCache<String,String>> appenderCache = new ConcurrentHashMap<String, LRUCache<String, String>>();
    private static final int APPENDER_LRUCACHE_SIZE = 500;

    public static void setCache(String key, String value) {
        if (appenderCache.containsKey(key)){
            LRUCache<String,String> temp = appenderCache.get(key);
            temp.put(value,null);
            appenderCache.put(key,temp);
        }else {
            LRUCache<String,String> cache = new LRUCache<String, String>(APPENDER_LRUCACHE_SIZE);
            cache.put(value,null);
            appenderCache.put(key,cache);
        }
    }

    public static Set<String> getCache(String key) {
        if (appenderCache.containsKey(key)){
            return appenderCache.get(key).getKeySet();
        }
        return null;
    }

}
