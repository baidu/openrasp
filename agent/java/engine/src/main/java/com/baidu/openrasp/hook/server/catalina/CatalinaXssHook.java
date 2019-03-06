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
import java.nio.ByteBuffer;
import java.util.HashMap;

/**
 * @author anyang
 * @Description: tomcat body_xss hook点
 * @date 2018/8/13 17:45
 */
@HookAnnotation
public class CatalinaXssHook extends ServerXssHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(CatalinaXssHook.class, "getCatalinaOutputBuffer", "$0", Object.class);
        insertBefore(ctClass, "close", "()V", src);

        String tomcat9Src = getInvokeStaticSrc(CatalinaXssHook.class, "getOutputBufferForTomcat9", "$1", ByteBuffer.class);
        insertBefore(ctClass, "realWriteBytes", null, tomcat9Src);
    }

    public static void getCatalinaOutputBuffer(Object object) {
        HashMap<String, Object> params = new HashMap<String, Object>();
        try {
            String content = null;
            String serverName = ApplicationModel.getServerName();
            String serverVersion = ApplicationModel.getVersion();
            if ("tomcat".equalsIgnoreCase(serverName)) {
                if (serverVersion.startsWith("6")) {
                    Object byteChunk = Reflection.getField(object, "bb");
                    byte[] buffer = (byte[]) Reflection.invokeMethod(byteChunk, "getBuffer", new Class[]{});
                    int start = (Integer) Reflection.invokeMethod(byteChunk, "getOffset", new Class[]{});
                    int len = (Integer) Reflection.invokeMethod(byteChunk, "getLength", new Class[]{});
                    if (len > 0) {
                        byte[] temp = new byte[len + 1];
                        System.arraycopy(buffer, start, temp, 0, len);
                        content = new String(temp);
                    }
                } else if (serverVersion.startsWith("7")) {
                    Object charChunk = Reflection.getField(object, "cb");
                    char[] buffer = (char[]) Reflection.invokeMethod(charChunk, "getBuffer", new Class[]{});
                    int start = (Integer) Reflection.invokeMethod(charChunk, "getOffset", new Class[]{});
                    int len = (Integer) Reflection.invokeMethod(charChunk, "getLength", new Class[]{});
                    if (len > 0) {
                        char[] temp = new char[len + 1];
                        System.arraycopy(buffer, start, temp, 0, len);
                        content = new String(temp);
                    }

                } else if (serverVersion.startsWith("8")) {
                    Object charBuffer = Reflection.getField(object, "cb");
                    content = charBuffer.toString();
                }

            } else if ("jboss".equalsIgnoreCase(serverName)) {
                if (serverVersion.startsWith("4")) {
                    Object byteChunk = Reflection.getField(object, "bb");
                    byte[] buffer = (byte[]) Reflection.getField(byteChunk, "buff");
                    int start = (Integer) Reflection.getField(byteChunk, "start");
                    int end = (Integer) Reflection.getField(byteChunk, "end");
                    if (end > start) {
                        byte[] temp = new byte[end - start + 1];
                        System.arraycopy(buffer, start, temp, 0, end - start);
                        content = new String(temp, "utf-8");
                    }
                } else if (serverVersion.startsWith("5") || serverVersion.startsWith("6")) {
                    Object charChunk = Reflection.getField(object, "cb");
                    char[] buffer = (char[]) Reflection.getField(charChunk, "buff");
                    int start = (Integer) Reflection.getField(charChunk, "start");
                    int end = (Integer) Reflection.getField(charChunk, "end");
                    if (end > start) {
                        char[] temp = new char[end - start + 1];
                        System.arraycopy(buffer, start, temp, 0, end - start);
                        content = new String(temp);
                    }
                }
            }
            params.put("html_body", content);

        } catch (Exception e) {
            String message = ApplicationModel.getServerName() + " xss detectde failed";
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
        if (HookHandler.requestCache.get() != null && !params.isEmpty()) {
            HookHandler.doCheck(CheckParameter.Type.XSS, params);
        }
    }

    public static void getOutputBufferForTomcat9(ByteBuffer buffer) {
        String serverName = ApplicationModel.getServerName();
        String serverVersion = ApplicationModel.getVersion();
        if ("tomcat".equalsIgnoreCase(serverName) && serverVersion.startsWith("9") && buffer != null) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                byte[] bytes = buffer.array();
                String content = new String(bytes);
                params.put("html_body", content);
            } catch (Exception e) {
                String message = ApplicationModel.getServerName() + " xss detectde failed";
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
            }
            if (HookHandler.requestCache.get() != null && !params.isEmpty()) {
                HookHandler.doCheck(CheckParameter.Type.XSS, params);
            }
        }
    }

}
