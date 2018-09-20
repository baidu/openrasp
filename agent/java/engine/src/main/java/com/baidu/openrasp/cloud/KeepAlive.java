package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.cloud.Utils.CloudUtils;
import com.baidu.openrasp.cloud.model.CloudCache;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.Map;

/**
 * @description: 创建rasp与云控的心跳线程并初始化rasp的config
 * @author: anyang
 * @create: 2018/09/17 16:55
 */
public class KeepAlive {
    private static final int KEEPALIVE_DELAY = 60000;

    static {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    Map<String, String> params = new HashMap<String, String>();
                    params.put("plugin_version", CloudCache.getCache("plugin_version"));
                    params.put("config_time", CloudCache.getCache("config_time"));
                    String content = new Gson().toJson(params);
                    String url = CloudRequestUrl.CLOUD_HEART_BEAT_URL;
                    String jsonString = new CloudHttp().request(url, content);
                    if (jsonString != null) {
                        GenericResponse response = new Gson().fromJson(jsonString, GenericResponse.class);
                        handler(response);
                    }
                    try {
                        Thread.sleep(KEEPALIVE_DELAY);
                    } catch (InterruptedException e) {
                       //continue next loop
                    }
                }
            }
        }).start();
    }

    private static void handler(GenericResponse response) {
        if (response.getResponseCode() >= 200 && response.getResponseCode() < 300 && response.getStatus() == 0) {
            Object configTime = response.getData().get("config_time");
            if (configTime != null) {
                CloudCache.setCache("config_time", String.valueOf(configTime));
            }
            Map pluginMap = CloudUtils.getMapFromResponse(response, "plugin");
            if (pluginMap != null) {
                CloudCache.setCache("plugin_version", String.valueOf(pluginMap.get("plugin_version")));
                CloudCache.setCache("plugin", String.valueOf(pluginMap.get("plugin")));
            }
            Map<String, Object> configMap = CloudUtils.getMapFromResponse(response, "config");
            if (configMap != null) {
                Config.getConfig().loadConfigFromCloud(configMap, true);
            }

        }
    }
}
