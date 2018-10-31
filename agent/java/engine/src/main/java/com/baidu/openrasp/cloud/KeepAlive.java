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
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.plugin.js.engine.JsPluginManager;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import org.mozilla.javascript.Scriptable;

import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

/**
 * @description: 创建rasp与云控的心跳线程并初始化rasp的config
 * @author: anyang
 * @create: 2018/09/17 16:55
 */
public class KeepAlive {
    private static final int KEEPALIVE_DELAY = 60000;

    public KeepAlive() {
        new Thread(new KeepAliveThread()).start();
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
                    CloudManager.LOGGER.warn("[OpenRASP] Cloud Control Send HeartBeat Failed");
                }
                try {
                    Thread.sleep(KEEPALIVE_DELAY);
                } catch (InterruptedException e) {
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
        String version = null;
        String md5 = null;
        String oldPlugin = null;
        String pluginContext = null;
        String algorithmConfig = null;
        Object configTime = CloudUtils.getValueFromData(response, "config_time");
        if (configTime instanceof JsonPrimitive) {
            CloudCacheModel.getInstance().setConfigTime(((JsonPrimitive) configTime).getAsLong());
        }
        Map<String, Object> pluginMap = CloudUtils.getMapFromData(response, "plugin");
        Map<String, Object> configMap = CloudUtils.getMapFromData(response, "config");
        if (pluginMap != null) {
            if (pluginMap.get("version") instanceof JsonPrimitive) {
                version = ((JsonPrimitive) pluginMap.get("version")).getAsString();
            }
            if (pluginMap.get("md5") instanceof JsonPrimitive) {
                md5 = ((JsonPrimitive) pluginMap.get("md5")).getAsString();
            }
            if (pluginMap.get("plugin") instanceof JsonPrimitive) {
                pluginContext = ((JsonPrimitive) pluginMap.get("plugin")).getAsString();
                try {
                    String pluginMD5 = CloudUtils.getMD5(pluginContext);
                    if (!pluginMD5.equals(md5)){
                       return;
                    }
                } catch (NoSuchAlgorithmException e) {
                    CloudManager.LOGGER.warn("Plugin MD5 Verification Failed: ",e);
                }
                oldPlugin = CloudCacheModel.getInstance().getPlugin();
                CloudCacheModel.getInstance().setPlugin(pluginContext);
                JsPluginManager.updatePluginAsync();
            }
        }
        if (configMap != null) {
            Object object = configMap.get("algorithm.config");
            if (object instanceof JsonObject) {
                JsonObject jsonObject = (JsonObject) object;
                algorithmConfig = new Gson().toJson(jsonObject);
            }
            Config.getConfig().loadConfigFromCloud(configMap, true);
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
        if (version != null && md5 != null && pluginContext != null && algorithmConfig != null) {
            boolean result = injectRhino(algorithmConfig);
            if (result) {
                CloudCacheModel.getInstance().setPluginVersion(version);
                CloudCacheModel.getInstance().setPluginMD5(md5);
                CloudCacheModel.getInstance().setAlgorithmConfig(algorithmConfig);
            } else {
                CloudCacheModel.getInstance().setPlugin(oldPlugin);
            }
        }
    }

    private static boolean injectRhino(String script) {
        try {
            script = "RASP.algorithmConfig =" + script;
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable globalScope = cx.getScope();
            cx.evaluateString(globalScope, script, "algorithmConfig", 1, null);
        } catch (Throwable e) {
            CloudManager.LOGGER.error("AlgorithmConfig Inject Rhino Failed,Plugin Will Be Invalid: ", e);
            JsPluginManager.disablePlugin();
            return false;
        }
        return true;
    }
}
