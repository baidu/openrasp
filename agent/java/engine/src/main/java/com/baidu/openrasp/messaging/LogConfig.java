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

package com.baidu.openrasp.messaging;

import com.baidu.openrasp.cloud.CloudManager;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;

import java.net.URI;

/**
 * Created by lxk on 17-4-10.
 * 用于导出日志配置
 */
public class LogConfig {

    /**
     * 初始化log4j的logger
     * 创建rasp.log、alarm.log、policy_alarm.log和plugin.log 的appender
     * 为appender增加限速
     */
    public static void ConfigFileAppender() throws Exception {
        DynamicConfigAppender.initLog4jLogger();
        DynamicConfigAppender.fileAppenderAddBurstFilter();
        System.out.println("[OpenRASP] Log4j initialized successfully");
    }

    /**
     * 管理syslog
     */
    public static void syslogManager() {
        if (Config.getConfig().getSyslogSwitch()) {
            String syslogUrl = Config.getConfig().getSyslogUrl();
            try {
                URI url = new URI(syslogUrl);
                String syslogAddress = url.getHost();
                int syslogPort = url.getPort();
                if (syslogAddress != null && !syslogAddress.trim().isEmpty() && syslogPort >= 0 && syslogPort <= 65535) {
                    DynamicConfigAppender.createSyslogAppender(syslogAddress, syslogPort);
                } else {
                    String message = "syslog url: " + syslogUrl + " is invalid";
                    int errorCode = ErrorType.CONFIG_ERROR.getCode();
                    CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode));
                }
            } catch (Exception e) {
                String message = "syslog url: " + syslogUrl + " parsed error";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                CloudManager.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }

        } else {
            DynamicConfigAppender.removeSyslogAppender();
        }
    }
}
