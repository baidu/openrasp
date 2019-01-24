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

package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.cloud.Register;
import com.baidu.openrasp.cloud.model.AppenderMappedLogger;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.tool.OSUtil;
import org.apache.log4j.Logger;

/**
 * Created by tyy on 18-8-10.
 *
 * 用于 hook 服务器的启动函数，用于记录服务器的基本信息，同时可以用作基线检测
 */
public abstract class ServerStartupHook extends AbstractClassHook {
    public static final Logger LOGGER = Logger.getLogger(ServerStartupHook.class.getName());

    @Override
    public String getType() {
        return "server_start";
    }

    /**
     * 开启云控时发送注册信息
     */
    protected static void sendRegister() {
        if (CloudUtils.checkCloudControlEnter()) {
            String cloudAddress = Config.getConfig().getCloudAddress();
            try {
                CloudCacheModel.getInstance().setMasterIp(OSUtil.getMasterIp(cloudAddress));
            } catch (Exception e) {
                String message = "get local ip failed";
                int errorCode = ErrorType.REGISTER_ERROR.getCode();
                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
            //初始化创建http appender
            DynamicConfigAppender.createRootHttpAppender();
            DynamicConfigAppender.createHttpAppender(AppenderMappedLogger.HTTP_ALARM.getLogger(),
                    AppenderMappedLogger.HTTP_ALARM.getAppender());
            DynamicConfigAppender.createHttpAppender(AppenderMappedLogger.HTTP_POLICY_ALARM.getLogger(),
                    AppenderMappedLogger.HTTP_POLICY_ALARM.getAppender());
            new Register();
        }
    }
}
