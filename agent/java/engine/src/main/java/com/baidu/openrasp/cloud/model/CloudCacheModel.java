/*
 * Copyright 2017-2021 Baidu Inc.
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

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.OSUtil;
import org.apache.commons.lang3.StringUtils;

/**
 * @description: 缓存云控数据
 * @author: anyang
 * @create: 2018/09/29 16:08
 */
public class CloudCacheModel {

    private static final int CACHE_SIZE = 500;

    public String plugin;
    public String pluginVersion = "";
    public String pluginMD5 = "";
    public long configTime = 0;
    public String algorithmConfig;
    public String raspId;
    public String pluginName = "";
    public static LRUCache<Long, Long> reportCache = new LRUCache<Long, Long>(CACHE_SIZE);

    private CloudCacheModel() {
    }

    private static class ObjectHolder {
        static CloudCacheModel instance = new CloudCacheModel();
    }

    public String getPluginMD5() {
        return pluginMD5;
    }

    public void setPluginMD5(String pluginMD5) {
        this.pluginMD5 = pluginMD5;
    }

    public String getRaspId() {
        return raspId;
    }

    public void setRaspId(String raspId) {
        this.raspId = raspId;
    }

    public String getPlugin() {
        return plugin;
    }

    public void setPlugin(String plugin) {
        this.plugin = plugin;
    }

    public String getPluginVersion() {
        return pluginVersion;
    }

    public void setPluginVersion(String pluginVersion) {
        this.pluginVersion = pluginVersion;
    }

    public long getConfigTime() {
        return configTime;
    }

    public void setConfigTime(long configTime) {
        this.configTime = configTime;
    }

    public String getAlgorithmConfig() {
        return algorithmConfig;
    }

    public void setAlgorithmConfig(String algorithmConfig) {
        this.algorithmConfig = algorithmConfig;
    }

    public String getMasterIp() {
        String ip = "";
        try {
            ip = OSUtil.getMasterIp(Config.getConfig().getCloudAddress());
        } catch (Exception e) {
            LogTool.warn(ErrorType.REGISTER_ERROR, "get local ip failed: " + e.getMessage(), e);
        }
        if (StringUtils.isEmpty(ip)) {
            LogTool.warn(ErrorType.REGISTER_ERROR, "get local ip failed, the local ip is empty");
        }
        return ip;
    }

    public String getPluginName() {
        return pluginName;
    }

    public void setPluginName(String pluginName) {
        this.pluginName = pluginName;
    }

    public static CloudCacheModel getInstance() {
        return ObjectHolder.instance;
    }
}
