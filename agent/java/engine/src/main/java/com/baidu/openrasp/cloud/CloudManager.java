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

import com.baidu.openrasp.cloud.Utils.CloudUtils;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.google.gson.Gson;
import org.apache.log4j.Logger;

import java.io.File;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

/**
 * @description: 初始化云控配置
 * @author: anyang
 * @create: 2018/09/18 15:09
 */
public class CloudManager {
    public static final Logger LOGGER = Logger.getLogger(CloudManager.class.getPackage().getName() + ".log");

    public static void init() throws Exception {
        if (CloudUtils.checkCloudControlEnter()) {
            Register.register();
            String content = new Gson().toJson(KeepAlive.GenerateParameters());
            String url = CloudRequestUrl.CLOUD_HEART_BEAT_URL;
            GenericResponse response = new CloudHttp().request(url, content);
            if (response != null && response.getStatus() != null && response.getStatus() == 0) {
                new KeepAlive();
            } else {
                System.out.println("[OpenRASP] Cloud Control Send HeartBeat Failed");
                throw new Exception();
            }
            new StatisticsReport();
        }
    }

}
