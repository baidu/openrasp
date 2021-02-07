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

package com.baidu.openrasp;

import com.baidu.openrasp.cloud.CloudManager;
import com.baidu.openrasp.cloud.model.CloudCacheModel;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.plugin.checker.CheckerManager;
import com.baidu.openrasp.plugin.js.JS;
import com.baidu.openrasp.tool.cpumonitor.CpuMonitorManager;
import com.baidu.openrasp.tool.model.BuildRASPModel;
import com.baidu.openrasp.transformer.CustomClassTransformer;
import com.baidu.openrasp.v8.CrashReporter;
import com.baidu.openrasp.v8.Loader;
import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;

import java.io.File;
import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;

/**
 * Created by tyy on 18-1-24.
 *
 * OpenRasp 引擎启动类
 */
public class EngineBoot implements Module {

    private CustomClassTransformer transformer;

    @Override
    public void start(String mode, Instrumentation inst) throws Exception {
        System.out.println("\n\n" +
                "   ____                   ____  ___   _____ ____ \n" +
                "  / __ \\____  ___  ____  / __ \\/   | / ___// __ \\\n" +
                " / / / / __ \\/ _ \\/ __ \\/ /_/ / /| | \\__ \\/ /_/ /\n" +
                "/ /_/ / /_/ /  __/ / / / _, _/ ___ |___/ / ____/ \n" +
                "\\____/ .___/\\___/_/ /_/_/ |_/_/  |_/____/_/      \n" +
                "    /_/                                          \n\n");
        try {
            Loader.load();
        } catch (Exception e) {
            System.out.println("[OpenRASP] Failed to load native library, please refer to https://rasp.baidu.com/doc/install/software.html#faq-v8-load for possible solutions.");
            e.printStackTrace();
            return;
        }
        if (!loadConfig()) {
            return;
        }
        //缓存rasp的build信息
        Agent.readVersion();
        BuildRASPModel.initRaspInfo(Agent.projectVersion, Agent.buildTime, Agent.gitCommit);
        // 初始化插件系统
        if (!JS.Initialize()) {
            return;
        }
        CheckerManager.init();
        initTransformer(inst);
        if (CloudUtils.checkCloudControlEnter()) {
            CrashReporter.install(Config.getConfig().getCloudAddress() + "/v1/agent/crash/report",
                    Config.getConfig().getCloudAppId(), Config.getConfig().getCloudAppSecret(),
                    CloudCacheModel.getInstance().getRaspId());
        }
        deleteTmpDir();
        String message = "[OpenRASP] Engine Initialized [" + Agent.projectVersion + " (build: GitCommit="
                + Agent.gitCommit + " date=" + Agent.buildTime + ")]";
        System.out.println(message);
        Logger.getLogger(EngineBoot.class.getName()).info(message);
    }

    @Override
    public void release(String mode) {
        CloudManager.stop();
        CpuMonitorManager.release();
        if (transformer != null) {
            transformer.release();
        }
        JS.Dispose();
        CheckerManager.release();
        String message = "[OpenRASP] Engine Released [" + Agent.projectVersion + " (build: GitCommit="
                + Agent.gitCommit + " date=" + Agent.buildTime + ")]";
        System.out.println(message);
    }

    private void deleteTmpDir() {
        try {
            File file = new File(Config.baseDirectory + File.separator + "jar_tmp");
            if (file.exists()) {
                FileUtils.deleteDirectory(file);
            }
        } catch (Throwable t) {
            Logger.getLogger(EngineBoot.class.getName()).warn("failed to delete jar_tmp directory: " + t.getMessage());
        }
    }

    /**
     * 初始化配置
     *
     * @return 配置是否成功
     */
    private boolean loadConfig() throws Exception {
        LogConfig.ConfigFileAppender();
        //单机模式下动态添加获取删除syslog
        if (!CloudUtils.checkCloudControlEnter()) {
            LogConfig.syslogManager();
        } else {
            System.out.println("[OpenRASP] RASP ID: " + CloudCacheModel.getInstance().getRaspId());
        }
        return true;
    }

    /**
     * 初始化类字节码的转换器
     *
     * @param inst 用于管理字节码转换器
     */
    private void initTransformer(Instrumentation inst) throws UnmodifiableClassException {
        transformer = new CustomClassTransformer(inst);
        transformer.retransform();
    }

}
