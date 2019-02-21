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

package com.baidu.openrasp.hook.server.wildfly;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerStartupHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by izpz on 19-1-31.
 * wildfly 启动 hook 点
 */
@HookAnnotation
public class UndertowStartupHook extends ServerStartupHook {
    /**
     * 用于判断类名与当前需要hook的类是否相同
     *
     * @param className 用于匹配的类名
     * @return 是否匹配
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/jboss/as/server/services/net/NetworkInterfaceService".equals(className);
    }

    /**
     * hook 目标类的函数
     *
     * @param ctClass 目标类
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(UndertowStartupHook.class, "handleUndertowStartup", "$0");
        insertBefore(ctClass, "start", null, src);
    }

    public static void handleUndertowStartup(Object server) {
        try {
            String version = "";
            ClassLoader classLoader = server.getClass().getClassLoader();
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Object moduleLoader = Reflection.invokeStaticMethod("org.jboss.modules.ModuleLoader",
                    "forClassLoader", new Class[]{ClassLoader.class}, classLoader);
            Object moduleIdentifier = Reflection.invokeStaticMethod("org.jboss.modules.ModuleIdentifier",
                    "create", new Class[]{String.class}, "org.jboss.as.version");
            if (moduleIdentifier != null) {
                Object module = Reflection.invokeMethod(moduleLoader, "loadModule",
                        new Class[]{moduleIdentifier.getClass()}, moduleIdentifier);

                ClassLoader moduleClassLoader = (ClassLoader) Reflection.invokeMethod(module, "getClassLoader", new Class[]{});
                if (moduleClassLoader != null) {
                    Class versionClass = moduleClassLoader.loadClass("org.jboss.as.version.Version");
                    version = (String) versionClass.getField("AS_VERSION").get(null);
                }
            }
            ApplicationModel.init("wildfly", version);
        } catch (Exception e) {
            HookHandler.LOGGER.warn("handle undertow startup failed", e);
        }
        sendRegister();
        if (!CloudUtils.checkCloudControlEnter()) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_RESIN_START, CheckParameter.EMPTY_MAP);
        }
    }
}
