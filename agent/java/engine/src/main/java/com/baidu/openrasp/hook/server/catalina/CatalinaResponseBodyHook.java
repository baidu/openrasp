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
import com.baidu.openrasp.hook.server.ServerResponseBodyHook;
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
public class CatalinaResponseBodyHook extends ServerResponseBodyHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/coyote/Response".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(CatalinaResponseBodyHook.class, "getBuffer", "$0,$1", Object.class, Object.class);
        insertBefore(ctClass, "doWrite", "(Lorg/apache/tomcat/util/buf/ByteChunk;)V", src);
        insertBefore(ctClass, "doWrite", "(Ljava/nio/ByteBuffer;)V", src);
    }

    public static void getBuffer(Object response, Object trunk) {
        boolean isCheckXss = isCheckXss();
        boolean isCheckSensitive = isCheckSensitive();
        if (HookHandler.isEnableXssHook() && (isCheckXss || isCheckSensitive) && trunk != null) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
                HttpServletResponse res = HookHandler.responseCache.get();
                String enc = null;
                String contentType = null;
                if (res != null) {
                    enc = res.getCharacterEncoding();
                    contentType = res.getContentType();
                }
                if (enc != null) {
                    params.put("buffer", trunk);
                    params.put("content_length", Reflection.invokeMethod(response, "getContentLength", new Class[]{}));
                    params.put("encoding", enc);
                    params.put("content_type", contentType);
                    if (trunk instanceof ByteBuffer) {
                        params.put("content", getContentFromByteBuffer((ByteBuffer) trunk, enc));
                    } else {
                        params.put("content", getContentFromByteTrunk(trunk, enc));
                    }
                    // 该处检测添加到 try catch 来捕捉拦截异常，XSS 检测不应该使用异常拦截，容易造成死循环
                    checkBody(params, isCheckXss, isCheckSensitive);
                }
            } catch (Exception e) {
                LogTool.traceHookWarn(ApplicationModel.getServerName() + " xss detection failed: " +
                        e.getMessage(), e);
            } finally {
                HookHandler.enableBodyXssHook();
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


    public static void handleXssBlockBuffer(CheckParameter parameter, String script) throws UnsupportedEncodingException {
        int contentLength = (Integer) parameter.getParam("content_length");
        Object buffer = parameter.getParam("buffer");
        byte[] content = script.getBytes(parameter.getParam("encoding").toString());
        if (buffer instanceof ByteBuffer) {
            ((ByteBuffer) buffer).clear();
        } else {
            Reflection.invokeMethod(buffer, "recycle", new Class[]{});
        }
        if (contentLength >= 0) {
            byte[] fullContent = new byte[contentLength];
            if (contentLength >= content.length) {
                for (int i = 0; i < content.length; i++) {
                    fullContent[i] = content[i];
                }
            }
            for (int i = content.length; i < contentLength; i++) {
                fullContent[i] = ' ';
            }
            writeContentToBuffer(buffer, fullContent);
        } else {
            writeContentToBuffer(buffer, content);
        }
    }

    private static void writeContentToBuffer(Object buffer, byte[] content) throws UnsupportedEncodingException {
        if (buffer instanceof ByteBuffer) {
            ByteBuffer b = (ByteBuffer) buffer;
            if (content.length > b.remaining() && b.remaining() > 0) {
                b.put((byte) ' ');
            } else {
                b.put(content, 0, content.length);
            }
            b.flip();
        } else {
            Reflection.invokeMethod(buffer, "setBytes", new Class[]{byte[].class, int.class, int.class},
                    content, 0, content.length);
        }
    }

}
