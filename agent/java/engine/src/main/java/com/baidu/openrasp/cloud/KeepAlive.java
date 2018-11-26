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

package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.plugin.js.engine.JsPluginManager;
import com.google.gson.Gson;
import com.google.gson.JsonPrimitive;

import java.util.HashMap;
import java.util.Map;

/**
 * @description: 创建rasp与云控的心跳线程并初始化rasp的config
 * @author: anyang
 * @create: 2018/09/17 16:55
 */
public class KeepAlive {

    public KeepAlive() {
        Thread thread = new Thread(new KeepAliveThread());
        thread.setDaemon(true);
        thread.start();
    }

    class KeepAliveThread implements Runnable {
        @Override
        public void run() {
            while (true) {
                String content = new Gson().toJson(GenerateParameters());
                String url = CloudRequestUrl.CLOUD_HEART_BEAT_URL;
                GenericResponse response = new CloudHttp().request(url, content);
                if (CloudUtils.checkRequestResult(response)) {
                    handleResponse(response);
                } else {
                    CloudManager.LOGGER.warn(CloudUtils.handleError(ErrorType.HEARTBEAT_ERROR, response));
                }
                try {
                    Thread.sleep(Config.getConfig().getHeartbeatInterval()*1000);
                } catch (Exception e) {
                    //continue next loop
                }
            }
        }
    }

    public static Map<String, Object> GenerateParameters() {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put("rasp_id", CloudCacheModel.getInstance().getRaspId());
        params.put("plugin_version", CloudCacheModel.getInstance().getPluginVersion());
        params.put("config_time", CloudCacheModel.getInstance().getConfigTime());
        params.put("plugin_md5", CloudCacheModel.getInstance().getPluginMD5());
        return params;
    }

    private static void handleResponse(GenericResponse response) {
        Long deliveryTime = null;
        String version = null;
        String md5 = null;
        String pluginContext = null;
        Object configTime = CloudUtils.getValueFromData(response, "config_time");
        Map<String, Object> pluginMap = CloudUtils.getMapFromData(response, "plugin");
        Map<String, Object> configMap = CloudUtils.getMapFromData(response, "config");
        if (configTime instanceof JsonPrimitive) {
            deliveryTime = ((JsonPrimitive) configTime).getAsLong();
        }
        if (pluginMap != null) {
            if (pluginMap.get("version") instanceof JsonPrimitive) {
                version = ((JsonPrimitive) pluginMap.get("version")).getAsString();
            }
            if (pluginMap.get("md5") instanceof JsonPrimitive) {
                md5 = ((JsonPrimitive) pluginMap.get("md5")).getAsString();
            }
            if (pluginMap.get("plugin") instanceof JsonPrimitive) {
                pluginContext = ((JsonPrimitive) pluginMap.get("plugin")).getAsString();
            }
        }
        if (configMap != null) {
            try {
                Config.getConfig().loadConfigFromCloud(configMap, true);
                if (deliveryTime != null) {
                    CloudCacheModel.getInstance().setConfigTime(deliveryTime);
                }
            } catch (Throwable e) {
                CloudManager.LOGGER.warn("config update failed: ", e);
            }
            if (configMap.get("log.maxburst") != null) {
                //更新http appender
                DynamicConfigAppender.createHttpAppender(DynamicConfigAppender.LOGGER_NAME,
                        DynamicConfigAppender.HTTP_ALARM_APPENDER_NAME);
                DynamicConfigAppender.createHttpAppender(DynamicConfigAppender.POLICY_LOGGER_NAME,
                        DynamicConfigAppender.HTTP_POLICY_APPENDER_NAME);
            }
            //云控下发配置时动态添加或者删除syslog
            Object syslogSwitch = configMap.get("syslog.enable");
            if (syslogSwitch != null) {
                LogConfig.syslogManager();
            }
            //云控下发配置时动态更新syslog.tag
            Object syslogTag = configMap.get("syslog.tag");
            if (syslogTag != null) {
                DynamicConfigAppender.updateSyslogTag();
            }
        }
        if (version != null && md5 != null && pluginContext != null) {
            JsPluginManager.updatePluginAsync(pluginContext, md5, version, deliveryTime);
        }
    }
}
