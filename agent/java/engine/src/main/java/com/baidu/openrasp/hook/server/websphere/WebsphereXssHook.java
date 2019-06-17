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
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;

/**
 * @author anyang
 * @Description: websphere的xss检测hook点
 * @date 2018/8/15 14:18
 */
@HookAnnotation
public class WebsphereXssHook extends ServerXssHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/wsspi/webcontainer/util/BufferedWriter".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WebsphereXssHook.class, "getWebsphereOutputBuffer", "$0", Object.class);
        insertBefore(ctClass, "flushChars", "()V", src);
    }

    public static void getWebsphereOutputBuffer(Object object) {
        if (HookHandler.isEnableXssHook()){
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                char[] buffer = (char[]) Reflection.getField(object, "buf");
                int len = (Integer) Reflection.getField(object, "count");
                char[] temp = new char[len];
                if (buffer != null) {
                    System.arraycopy(buffer, 0, temp, 0, len);
                    String content = new String(temp);
                    params.put("html_body", content);
                }
            } catch (Exception e) {
                String message = ApplicationModel.getServerName() + " xss detectde failed";
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
            if (isCheckXss() && !params.isEmpty()) {
                HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
            }
        }
    }
}
