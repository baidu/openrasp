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

package com.baidu.openrasp;

import java.lang.instrument.Instrumentation;

import static com.baidu.openrasp.Module.*;

/**
 * Created by tyy on 3/27/17.
 * 加载agent的入口类，先于主函数加载
 */
public class Agent {

    /**
     * 启动时加载的agent入口方法
     *
     * @param agentArg 启动参数
     * @param inst     {@link Instrumentation}
     */
    public static void premain(String agentArg, Instrumentation inst) {
        init(START_MODE_NORMAL, START_ACTION_INSTALL, inst);
    }

    /**
     * attack 机制加载 agent
     *
     * @param agentArg 启动参数
     * @param inst     {@link Instrumentation}
     */
    public static void agentmain(String agentArg, Instrumentation inst) {
        init(Module.START_MODE_ATTACH, agentArg, inst);
    }

    /**
     * attack 机制加载 agent
     *
     * @param mode 启动模式
     * @param inst {@link Instrumentation}
     */
    public static synchronized void init(String mode, String action, Instrumentation inst) {
        try {
            JarFileHelper.addJarToBootstrap(inst);
            ModuleLoader.load(mode, action, inst);
        } catch (Throwable e) {
            System.err.println("[OpenRASP] Failed to initialize, will continue without security protection.");
            e.printStackTrace();
        }
    }

}