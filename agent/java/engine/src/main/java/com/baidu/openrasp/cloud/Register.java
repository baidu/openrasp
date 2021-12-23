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

package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.model.CloudRequestUrl;
import com.baidu.openrasp.cloud.model.GenericResponse;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.OSUtil;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.baidu.openrasp.tool.model.BuildRASPModel;
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
    private RegisterCallback callback;

    public Register(RegisterCallback callback) {
        this.callback = callback;
        Thread thread = new Thread(new RegisterThread());
        thread.setName("OpenRASP Register Thread");
        thread.setDaemon(true);
        thread.start();
    }

    public static interface RegisterCallback {
        void call();
    }

    class RegisterThread implements Runnable {
        private boolean registerFlag = false;

        @Override
        public void run() {
            while (!this.registerFlag) {
                try {
                    String content = new Gson().toJson(generateParameters());
                    String url = CloudRequestUrl.CLOUD_REGISTER_URL;
                    GenericResponse response = new CloudHttp().commonRequest(url, content);
                    if (CloudUtils.checkResponse(response)) {
                        this.registerFlag = true;
                        Config.getConfig().setHookWhiteAll("false");
                        System.out.println("[OpenRASP] RASP agent successfully registered, enabling remote management, please refer to rasp logs for details");
                        CloudManager.LOGGER.info("[OpenRASP] RASP agent successfully registered, registration details are as follows: \n" + content);
                        callback.call();
                    } else {
                        System.out.println("[OpenRASP] Failed to register RASP agent, please refer to rasp logs for details");
                        String message = CloudUtils.handleError(ErrorType.REGISTER_ERROR, response);
                        LogTool.warn(ErrorType.REGISTER_ERROR, message);
                    }
                } catch (Throwable e) {
                    LogTool.warn(ErrorType.REGISTER_ERROR, e.getMessage(), e);
                }

                try {
                    Thread.sleep(REGISTER_DELAY);
                } catch (InterruptedException e) {
                    LogTool.warn(ErrorType.REGISTER_ERROR, e.getMessage(), e);
                }
            }
        }
    }

    private static Map<String, Object> generateParameters() {
        Map<String, Object> params = new HashMap<String, Object>();
        params.put("id", CloudCacheModel.getInstance().getRaspId());
        params.put("version", BuildRASPModel.getRaspVersion());
        params.put("hostname", OSUtil.getHostName());
        params.put("os", OSUtil.getOs());
        params.put("language", "java");
        params.put("language_version", System.getProperty("java.version"));
        params.put("server_type", ApplicationModel.getServerName());
        params.put("server_version", ApplicationModel.getVersion());
        params.put("rasp_home", Config.getConfig().getBaseDirectory());
        params.put("register_ip", CloudCacheModel.getInstance().getMasterIp());
        params.put("heartbeat_interval", Config.getConfig().getHeartbeatInterval());
        params.put("environ", ApplicationModel.getSystemEnv());
        String VMType = ApplicationModel.getVMType();
        if (VMType != null) {
            params.put("host_type", VMType);
        }
        return params;
    }
}
