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

package com.baidu.openrasp.hook.server.catalina;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerStartupHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 8/9/17.
 * 用于hook tomcat启动函数
 */
@HookAnnotation
public class TomcatStartupHook extends ServerStartupHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/startup/Catalina".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(TomcatStartupHook.class, "checkTomcatStartup", "");
        insertBefore(ctClass, "start", null, src);
    }

    /**
     * tomcat启动时检测安全规范
     */
    public static void checkTomcatStartup() {
        try {
            String serverInfo = (String) Reflection.invokeStaticMethod("org.apache.catalina.util.ServerInfo",
                    "getServerInfo", new Class[]{});
            if (serverInfo != null && serverInfo.toLowerCase().contains("tomcat")) {
                String version = (String) Reflection.invokeStaticMethod(
                        "org.apache.catalina.util.ServerInfo", "getServerNumber", new Class[]{});
                ApplicationModel.init("tomcat", version);
            }
        } catch (Exception e) {
            HookHandler.LOGGER.warn("handle tomcat startup failed", e);
        }
        sendRegister();
        if (!CloudUtils.checkCloudControlEnter()){
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_TOMCAT_START, CheckParameter.EMPTY_MAP);
        }
    }

}
