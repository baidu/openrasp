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

package com.baidu.openrasp.hook.server.weblogic;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
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
 * @author anyang
 * @Description: weblogic启动hook点
 * @date 2018/8/27 11:50
 */
@HookAnnotation
public class WeblogicStartupHook extends ServerStartupHook {
    @Override
    public boolean isClassMatched(String className) {
        return "weblogic/t3/srvr/T3Srvr".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WeblogicStartupHook.class, "handleWeblogicStartup", "$0", Object.class);
        insertBefore(ctClass, "startup", "()V", src);
    }

    public static void handleWeblogicStartup(Object sevrer) {
        try {
            Class clazz = sevrer.getClass().getClassLoader().loadClass("weblogic.version");
            String version = (String) Reflection.invokeStaticMethod(clazz.getName(), "getVersions", new Class[]{});
            if (version != null) {
                int index = getFirstNumIndexFromString(version);
                if (index >= 0) {
                    version = version.substring(index);
                    ApplicationModel.init("weblogic", version);
                }
            }
        } catch (Exception e) {
            String message = "handle weblogic startup failed";
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        sendRegister();
        if (!CloudUtils.checkCloudControlEnter()) {
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.POLICY_WEBLOGIC_START, CheckParameter.EMPTY_MAP);
        }
    }

    private static int getFirstNumIndexFromString(String version) {
        for (int i = 0; i < version.length(); i++) {
            if (version.charAt(i) >= '0' && version.charAt(i) <= '9') {
                return i;
            }
        }
        return -1;
    }
}
