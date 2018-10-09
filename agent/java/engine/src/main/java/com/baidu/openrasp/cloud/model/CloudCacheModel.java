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

import java.util.HashMap;

/**
 * @description: 缓存云控数据
 * @author: anyang
 * @create: 2018/09/29 16:08
 */
public class CloudCacheModel {
    public String plugin;
    public String pluginVersion = "0";
    public long configTime = 0;
    public String algorithmConfig;
    public String raspId;
    public static HashMap<Long,Long> reportCache = new HashMap<Long, Long>();

    private CloudCacheModel() {
    }

    private static class ObjectHolder {
        static CloudCacheModel instance = new CloudCacheModel();
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

    public static CloudCacheModel getInstance() {
        return ObjectHolder.instance;
    }
}
