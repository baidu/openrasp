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

package com.baidu.openrasp.tool.model;

import java.util.HashMap;

/**
 * Created by tyy on 18-8-13.
 *
 * 缓存服务器进程基本信息
 */
public class ApplicationModel {

    private static HashMap<String, String> applicationInfo = new HashMap<String, String>(8);

    public static void init(String serverName, String version) {
        applicationInfo.put("server", serverName);
        applicationInfo.put("version", version);
        applicationInfo.put("os", System.getProperty("os.name"));
        applicationInfo.put("language", "java");
    }

    public static HashMap<String, String> getApplicationInfo() {
        return applicationInfo;
    }

    public static String getVersion() {
        return applicationInfo.get("version");
    }

    public static String getServerName() {
        return applicationInfo.get("server");
    }

    public static String getRaspVersion(){
        return applicationInfo.get("projectVersion");
    }

}
