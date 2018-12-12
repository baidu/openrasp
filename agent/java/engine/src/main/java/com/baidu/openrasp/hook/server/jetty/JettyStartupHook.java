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

package com.baidu.openrasp.hook.server.jetty;

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
 * Created by tyy on 18-8-10.
 *
 * jetty 启动 hook 点
 */
@HookAnnotation
public class JettyStartupHook extends ServerStartupHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/eclipse/jetty/server/Server".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(JettyStartupHook.class, "handleJettyStartup", "$0");
        insertAfter(ctClass, "doStart", null, src, true);
    }

    public static void handleJettyStartup(Object server) {
        try {
            ApplicationModel.init("jetty",
                    Reflection.invokeStringMethod(server, "getVersion", new Class[]{}));
        } catch (Exception e) {
            HookHandler.LOGGER.warn("handle resin startup failed", e);
        }
        sendRegister();
        if (!CloudUtils.checkCloudControlEnter()){
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_JETTY_START, CheckParameter.EMPTY_MAP);
        }
    }
}
