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
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.google.gson.Gson;

import java.util.HashMap;
import java.util.Map;

/**
 * @description: 云控注册接口
 * @author: anyang
 * @create: 2018/10/08 11:58
 */
public class Register {
    private static final int REGISTER_DELAY = 300 * 1000;

    public Register() {
        Thread thread = new Thread(new RegisterThread());
        thread.setDaemon(true);
        thread.start();
    }

    class RegisterThread implements Runnable {
        private boolean registerFlag = false;

        @Override
        public void run() {
            while (!this.registerFlag) {
                String content = new Gson().toJson(GenerateParameters());
                String url = CloudRequestUrl.CLOUD_REGISTER_URL;
                GenericResponse response = new CloudHttp().request(url, content);
                if (CloudUtils.checkRequestResult(response)) {
                    this.registerFlag = true;
                    Config.getConfig().setHookWhiteAll("false");
                    System.out.println("[OpenRASP] RASP agent successfully registered, enabling remote management");
                    CloudManager.init();
                } else {
                    System.out.println("[OpenRASP] Failed to register RASP agent, please refer to rasp logs for details");
                    CloudManager.LOGGER.warn(CloudUtils.handleError(ErrorType.REGISTER_ERROR, response));
                }
                try {
                    Thread.sleep(REGISTER_DELAY);
                } catch (InterruptedException e) {
                    //continue next loop
                }
            }
        }
    }

    private static Map<String, Object> GenerateParameters() {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put("id", CloudCacheModel.getInstance().getRaspId());
        params.put("version", ApplicationModel.getRaspVersion());
        params.put("hostname", OSUtil.getHostName());
        params.put("language", "java");
        params.put("language_version", System.getProperty("java.version"));
        params.put("server_type", ApplicationModel.getServerName());
        params.put("server_version", ApplicationModel.getVersion());
        String raspHome = Config.getConfig().getBaseDirectory();
        params.put("rasp_home", raspHome);
        params.put("register_ip", CloudCacheModel.getInstance().getMasterIp());
        int heartbeatInterval = Config.getConfig().getHeartbeatInterval();
        params.put("heartbeat_interval",heartbeatInterval);
        return params;
    }
}
