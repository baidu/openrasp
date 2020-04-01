/*
 * Copyright 2017-2020 Baidu Inc.
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

package com.baidu.openrasp.detector;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.CloudManager;
import com.baidu.openrasp.cloud.Register;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.cpumonitor.CpuMonitorManager;
import com.baidu.openrasp.tool.model.ApplicationModel;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.io.FileReader;
import java.net.URLDecoder;
import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-12.
 *
 * 服务器信息探测
 */
public abstract class ServerDetector {

    private static final Logger LOGGER = Logger.getLogger(ServerDetector.class.getName());

    /**
     * 探测该类是否为服务器标志类
     *
     * @param className   类名
     * @param classLoader 类的加载器
     */
    public synchronized boolean handleServer(String className, ClassLoader classLoader, ProtectionDomain domain) {
        boolean isDetected = handleServerInfo(classLoader, domain);
        if (isDetected) {
            HookHandler.enableHook.set(true);
            sendRegister();
            CpuMonitorManager.start();
        }
        return isDetected;
    }

    public abstract boolean isClassMatched(String className);

    public abstract boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain);

    protected void sendRegister() {
        if (CloudUtils.checkCloudControlEnter()) {
            new Register(new Register.RegisterCallback() {
                @Override
                public void call() {
                    CloudManager.init();
                }
            });
        } else {
            // 避免基线检测在 transformer 线程中造成提前加载需要 hook 的类
            Thread policyThread = new Thread() {
                @Override
                public void run() {
                    checkServerPolicy();
                }
            };
            policyThread.setDaemon(true);
            policyThread.start();
        }
    }

    /**
     * 服务器基线检测
     */
    public synchronized static void checkServerPolicy() {
        try {
            String serverName = ApplicationModel.getServerName();
            if ("tomcat".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_TOMCAT, CheckParameter.EMPTY_MAP);
            } else if ("jboss".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_JBOSS, CheckParameter.EMPTY_MAP);
            } else if ("jetty".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_JETTY, CheckParameter.EMPTY_MAP);
            } else if ("resin".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_RESIN, CheckParameter.EMPTY_MAP);
            } else if ("websphere".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_WEBSPHERE, CheckParameter.EMPTY_MAP);
            } else if ("weblogic".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_WEBLOGIC, CheckParameter.EMPTY_MAP);
            } else if ("undertow".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_WILDFLY, CheckParameter.EMPTY_MAP);
            } else if ("jboss eap".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_JBOSSEAP, CheckParameter.EMPTY_MAP);
            } else if ("tongweb".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_TONGWEB, CheckParameter.EMPTY_MAP);
            } else if ("bes".equals(serverName)) {
                HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SERVER_BES, CheckParameter.EMPTY_MAP);
            }
        } catch (Throwable t) {
            LogTool.warn(ErrorType.HOOK_ERROR, "failed to do server policy checking: " + t.getMessage(), t);
        }
    }

    public void logDetectError(String message, Throwable t) {
        LogTool.warn(ErrorType.DETECT_SERVER_ERROR, message, t);
    }

    public boolean isWildfly(ProtectionDomain domain) {
        try {
            String path = domain.getCodeSource().getLocation().getFile();
            path = URLDecoder.decode(path, "UTF-8");
            File runJar = new File(path);
            File homeFile = runJar.getParentFile();
            File dir = new File(homeFile.getCanonicalPath() + File.separator + "bin" + File.separator + "init.d");
            if (dir.exists() && dir.isDirectory()) {
                File[] files = dir.listFiles(new FileFilter() {
                    @Override
                    public boolean accept(File file) {
                        return file.getName().contains("wildfly");
                    }
                });
                return files != null && files.length > 0;
            } else {
                return detectWildfly(homeFile.getCanonicalPath());
            }
        } catch (Exception e) {
            logDetectError("identified wildfly and jboss eap failed", e);
        }
        return false;
    }

    private boolean detectWildfly(String severRoot) throws Exception {
        File baseDir = new File(severRoot);
        if (baseDir.exists() && baseDir.isDirectory()) {
            String content = IOUtils.toString(new FileReader(new File(baseDir.getCanonicalPath() + File.separator + "README.txt")));
            return content != null && content.toLowerCase().contains("wildfly");
        }
        return false;
    }
}
