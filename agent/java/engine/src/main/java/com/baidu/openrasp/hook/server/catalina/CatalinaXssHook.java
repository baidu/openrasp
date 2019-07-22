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
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.lang.reflect.Field;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
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
        String src = getInvokeStaticSrc(CatalinaXssHook.class, "getBuffer", "$0", Object.class);
        insertBefore(ctClass, "close", "()V", src);
    }

    public static void getBuffer(Object out) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            if (out != null) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                try {
                    Object buffer = getField(out, "cb");
                    if (buffer instanceof CharBuffer) {
                        String content = getContentFromCharBuffer(buffer);
                        if (!StringUtils.isEmpty(content)) {
                            params.put("html_body", content);
                        }
                    }
                    if (params.isEmpty() && buffer != null && !isBuffer(buffer)) {
                        String content = getContentFromCharChunk(buffer);
                        if (!StringUtils.isEmpty(content)) {
                            params.put("html_body", content);
                        }
                    }
                    if (params.isEmpty()) {
                        buffer = getField(out, "bb");
                        if (buffer instanceof ByteBuffer) {
                            String content = getContentFromByteBuffer(buffer);
                            if (!StringUtils.isEmpty(content)) {
                                params.put("html_body", content);
                            }
                        }
                        if (params.isEmpty() && buffer != null && !isBuffer(buffer) ) {
                            String content = getContentFromByteChunk(buffer);
                            if (!StringUtils.isEmpty(content)) {
                                params.put("html_body", content);
                            }
                        }
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

    public static String getContentFromCharBuffer(Object buffer) {
        return ((CharBuffer) buffer).toString();
    }

    public static String getContentFromByteBuffer(Object buffer) {
        byte[] bytes = ((ByteBuffer) buffer).array();
        return new String(bytes);
    }

    public static String getContentFromByteChunk(Object buffer) {
        byte[] bytes = (byte[]) Reflection.invokeMethod(buffer, "getBuffer", new Class[]{});
        int start = (Integer) Reflection.invokeMethod(buffer, "getOffset", new Class[]{});
        int len = (Integer) Reflection.invokeMethod(buffer, "getLength", new Class[]{});
        byte[] temp = new byte[len + 1];
        System.arraycopy(bytes, start, temp, 0, len);
        return new String(temp);
    }

    public static String getContentFromCharChunk(Object buffer) {
        char[] chars = (char[]) Reflection.invokeMethod(buffer, "getBuffer", new Class[]{});
        int start = (Integer) Reflection.invokeMethod(buffer, "getOffset", new Class[]{});
        int len = (Integer) Reflection.invokeMethod(buffer, "getLength", new Class[]{});
        char[] temp = new char[len + 1];
        System.arraycopy(chars, start, temp, 0, len);
        return new String(temp);
    }

    public static Object getField(Object object, String fieldName) {
        try {
            Field field = object.getClass().getDeclaredField(fieldName);
            field.setAccessible(true);
            return field.get(object);
        } catch (Exception e) {
            return null;
        }
    }

    public static boolean isBuffer(Object buffer) {
        return buffer instanceof ByteBuffer || buffer instanceof CharBuffer;
    }
}