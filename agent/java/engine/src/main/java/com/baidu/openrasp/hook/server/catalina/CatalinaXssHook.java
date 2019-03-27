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
        String src1 = getInvokeStaticSrc(CatalinaXssHook.class, "getBufferFromByteArray", "$1,$2,$3", byte[].class, int.class, int.class);
        String src2 = getInvokeStaticSrc(CatalinaXssHook.class, "getBufferFromByteBuffer", "$1", ByteBuffer.class);
        insertBefore(ctClass, "realWriteBytes", "([BII)V", src1);
        insertBefore(ctClass, "realWriteBytes", "(Ljava/nio/ByteBuffer;)V", src2);
    }

    public static void getBufferFromByteArray(byte[] buf, int off, int cnt) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            if (buf != null && cnt > 0) {
                try {
                    byte[] temp = new byte[cnt + 1];
                    System.arraycopy(buf, off, temp, 0, cnt);
                    String content = new String(temp);
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

    public static void getBufferFromByteBuffer(ByteBuffer buffer) {
        if (HookHandler.isEnableXssHook()) {
            HookHandler.disableBodyXssHook();
            if (buffer != null) {
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
                    HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
                }
            }
        }
    }
}
