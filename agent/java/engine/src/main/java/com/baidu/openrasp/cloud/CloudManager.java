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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.apache.log4j.Logger;

/**
 * @description: 初始化云控配置
 * @author: anyang
 * @create: 2018/09/18 15:09
 */
public class CloudManager {
    public static final Logger LOGGER = Logger.getLogger(CloudManager.class.getPackage().getName() + ".log");

    public static void init() {
        checkServerPolicy();
        new KeepAlive();
        new StatisticsReport();
    }

    /**
     * 服务器基线检测
     */
    private static void checkServerPolicy() {
        String serverName = ApplicationModel.getServerName();
        if ("tomcat".equals(serverName)) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_TOMCAT_START, CheckParameter.EMPTY_MAP);
        } else if ("jboss".equals(serverName)) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_JBOSS_START, CheckParameter.EMPTY_MAP);
        } else if ("jetty".equals(serverName)) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_JETTY_START, CheckParameter.EMPTY_MAP);
        } else if ("resin".equals(serverName)) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_RESIN_START, CheckParameter.EMPTY_MAP);
        } else if ("websphere".equals(serverName)) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_WEBSPHERE_START, CheckParameter.EMPTY_MAP);
        }
    }

}
