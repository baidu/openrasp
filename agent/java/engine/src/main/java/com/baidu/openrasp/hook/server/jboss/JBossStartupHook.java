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

package com.baidu.openrasp.hook.server.jboss;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.AbstractClassHook;
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
 * 　　* @Description: JBoss的基线检查
 * 　　* @author anyang
 * 　　* @date 2018/7/30 15:46
 */
@HookAnnotation
public class JBossStartupHook extends ServerStartupHook {


    @Override
    public boolean isClassMatched(String className) {
        return "org/jboss/system/server/ServerImpl".equals(className) ||
                "org/jboss/bootstrap/AbstractServerImpl".equals(className) ||
                "org/jboss/bootstrap/impl/base/server/AbstractServer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(JBossStartupHook.class, "checkJBossJMXConsole", "$0", Object.class);
        insertBefore(ctClass, "start", null, src);
    }

    /**
     * JBoss启动时检测JMX Console配置
     */
    public static void checkJBossJMXConsole(Object object) {

        try {
            String serverVersion = Reflection.invokeStringMethod(object, "getVersionNumber", new Class[]{});
            if (serverVersion != null) {
                ApplicationModel.init("jboss", serverVersion);
            }
        } catch (Exception e) {
            String message = "handle jboss startup failed";
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        if (!CloudUtils.checkCloudControlEnter()) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_JBOSS_START, CheckParameter.EMPTY_MAP);
        }
    }
}
