package com.baidu.openrasp.cloud.model;

import java.util.HashMap;

/**
 * @program openrasp
 * @description: 缓存云控参数
 * @author: anyang
 * @create: 2018/09/17 17:28
 */
public class CloudCache {
    private static HashMap<String, String> parameters = new HashMap<String, String>();
    static {
        parameters.put("plugin_version","0");
        parameters.put("config_time","0");
    }

    public static void setCache(String key, String value) {
        parameters.put(key, value);
    }

    public static String getCache(String key) {
        return parameters.get(key);
    }
}
