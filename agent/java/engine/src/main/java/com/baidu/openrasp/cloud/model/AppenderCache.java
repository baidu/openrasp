package com.baidu.openrasp.cloud.model;

import com.baidu.openrasp.tool.LRUCache;

import java.util.HashMap;
import java.util.Set;

/**
 * @description: 缓存需要重传的日志
 * @author: anyang
 * @create: 2018/09/20 13:59
 */
public class AppenderCache {
    private static HashMap<String, LRUCache<String,String>> appenderCache = new HashMap<String, LRUCache<String, String>>();
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
