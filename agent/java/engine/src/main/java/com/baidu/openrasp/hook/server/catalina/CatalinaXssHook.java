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
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.response.HttpServletResponse;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
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
        return "org/apache/coyote/Response".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(CatalinaXssHook.class, "getBuffer", "$1", Object.class);
        insertBefore(ctClass, "doWrite", "(Lorg/apache/tomcat/util/buf/ByteChunk;)V", src);
        insertBefore(ctClass, "doWrite", "(Ljava/nio/ByteBuffer;)V", src);
    }

    public static void getBuffer(Object trunk) {
        if (HookHandler.isEnableXssHook() && isCheckXss() && trunk != null) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                HttpServletResponse res = HookHandler.responseCache.get();
                String enc = null;
                if (res != null) {
                    enc = res.getCharacterEncoding();
                }
                if (enc != null) {
                    if (trunk instanceof ByteBuffer) {
                        params.put("html_body", getContentFromByteBuffer((ByteBuffer) trunk, enc));
                    } else {
                        params.put("html_body", getContentFromByteTrunk(trunk, enc));
                    }
                }
            } catch (Exception e) {
                LogTool.traceHookWarn(ApplicationModel.getServerName() + " xss detectde failed: " +
                        e.getMessage(), e);
                return;
            }
            if (!params.isEmpty()) {
                HookHandler.doCheck(CheckParameter.Type.XSS_USERINPUT, params);
            }

        }
    }

    private static String getContentFromByteBuffer(ByteBuffer trunk, String enc) throws UnsupportedEncodingException {
        byte[] bytes = trunk.array();
        int end = trunk.limit();
        int start = trunk.position();
        byte[] tmp = new byte[end - start];
        System.arraycopy(bytes, start, tmp, 0, end - start);
        return new String(tmp, enc);
    }

    private static String getContentFromByteTrunk(Object trunk, String enc) throws UnsupportedEncodingException {
        byte[] bytes = (byte[]) Reflection.invokeMethod(trunk, "getBuffer", new Class[]{});
        int start = (Integer) Reflection.invokeMethod(trunk, "getStart", new Class[]{});
        int end = (Integer) Reflection.invokeMethod(trunk, "getEnd", new Class[]{});
        byte[] tmp = new byte[end - start];
        System.arraycopy(bytes, start, tmp, 0, end - start);
        return new String(tmp, enc);
    }

}