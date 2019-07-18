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

package com.baidu.openrasp.cloud;

import com.baidu.openrasp.cloud.model.AppenderMappedLogger;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.detector.ServerDetector;
import org.apache.log4j.Logger;

import java.util.LinkedList;

/**
 * @description: 初始化云控配置
 * @author: anyang
 * @create: 2018/09/18 15:09
 */
public class CloudManager {
    public static final Logger LOGGER = Logger.getLogger(CloudManager.class.getPackage().getName() + ".log");

    private static LinkedList<CloudTimerTask> tasks = new LinkedList<CloudTimerTask>();

    public static void init() {
        //注册成功之后初始化创建http appender
        DynamicConfigAppender.createRootHttpAppender();
        DynamicConfigAppender.createHttpAppender(AppenderMappedLogger.HTTP_ALARM.getLogger(),
                AppenderMappedLogger.HTTP_ALARM.getAppender());
        DynamicConfigAppender.createHttpAppender(AppenderMappedLogger.HTTP_POLICY_ALARM.getLogger(),
                AppenderMappedLogger.HTTP_POLICY_ALARM.getAppender());
        ServerDetector.checkServerPolicy();
        tasks.add(new KeepAlive());
        tasks.add(new StatisticsReport());
        for (CloudTimerTask task : tasks) {
            task.start();
        }
    }

    public static void stop() {
        for (CloudTimerTask task : tasks) {
            task.stop();
        }
    }

}
