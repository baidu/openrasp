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

package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.HookHandler;
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
 * @Description: 获取websphere的serverInfo
 * @date 2018/8/14 17:08
 */
@HookAnnotation
public class WebsphereStartupHook extends ServerStartupHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/websphere/product/WASProduct".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WebsphereStartupHook.class, "handleWebsphereStartup", "$_");
        insertAfter(ctClass, "getPlatform", null, src);
    }

    public static void handleWebsphereStartup(Object websphere) {
        try {
            String serverVersion = (String) Reflection.invokeMethod(websphere, "getVersion", new Class[]{});
            ApplicationModel.init("websphere",serverVersion);
        } catch (Exception e) {
            HookHandler.LOGGER.warn("handle websphere startup failed", e);
        }
        HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_WEBSPHERE_START, CheckParameter.EMPTY_MAP);
    }
}
