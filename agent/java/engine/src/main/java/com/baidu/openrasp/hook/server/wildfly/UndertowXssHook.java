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

package com.baidu.openrasp.hook.server.wildfly;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.nio.CharBuffer;
import java.util.HashMap;

/**
 * @description: undertow body-xss hook点
 * @author: anyang
 * @create: 2019/02/22 11:27
 */
@HookAnnotation
public class UndertowXssHook extends ServerXssHook {
    @Override
    public boolean isClassMatched(String className) {
        return "io/undertow/servlet/spec/ServletPrintWriter".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(UndertowXssHook.class, "getUndertowOutputBuffer", "$1", CharBuffer.class);
        String src2 = getInvokeStaticSrc(UndertowXssHook.class, "getUndertowOutputBuffer", "$1,$2,$3", String.class, int.class, int.class);
        insertBefore(ctClass, "write", "(Ljava/nio/CharBuffer;)V", src1);
        insertBefore(ctClass, "write", "(Ljava/lang/String;II)V", src2);
    }

    public static void getUndertowOutputBuffer(CharBuffer buffer) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            if (buffer != null) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                try {
                    String content = buffer.toString();
                    params.put("html_body", content);
                } catch (Exception e) {
                    String message = ApplicationModel.getServerName() + " xss detectde failed";
                    int errorCode = ErrorType.HOOK_ERROR.getCode();
                    HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                }
                if (HookHandler.requestCache.get() != null && !params.isEmpty()) {
                    HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
                }
            }
        }
    }

    public static void getUndertowOutputBuffer(String buffer, int off, int len) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            if (buffer != null) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                params.put("html_body", buffer);
                if (isCheckXss() && !params.isEmpty()) {
                    HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
                }
            }
        }
    }

}
