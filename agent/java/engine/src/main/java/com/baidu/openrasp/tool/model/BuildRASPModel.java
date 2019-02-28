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
 * @description: 缓存RASP的build信息
 * @author: anyang
 * @create: 2019/02/18 11:22
 */
public class BuildRASPModel {
    private static HashMap<String, String> raspInfo;

    static {
        raspInfo = new HashMap<String, String>(3);
        raspInfo.put("projectVersion", "");
        raspInfo.put("buildTime", "");
        raspInfo.put("gitCommit", "");
    }

    public static void initRaspInfo(String projectVersion, String buildTime, String gitCommit) {
        projectVersion = (projectVersion == null ? "" : projectVersion);
        buildTime = (buildTime == null ? "" : buildTime);
        gitCommit = (gitCommit == null ? "" : gitCommit);
        raspInfo.put("projectVersion", projectVersion);
        raspInfo.put("buildTime", buildTime);
        raspInfo.put("gitCommit", gitCommit);
    }

    public static HashMap<String, String> getRaspInfo() {
        return raspInfo;
    }

    public static String getRaspVersion() {
        return raspInfo.get("projectVersion");
    }

    public static String getBuildTime() {
        return raspInfo.get("buildTime");
    }

    public static String getGitCommit() {
        return raspInfo.get("gitCommit");
    }

}
