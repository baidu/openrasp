/**
 *
 */
package com.baidu.openrasp.hook.server.bes;

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
 * @description: BES response body hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESResponseBodyHook extends ServerResponseBodyHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/bes/enterprise/webtier/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(BESResponseBodyHook.class, "getBuffer", "$0,$1", Object.class, Object.class);
        insertBefore(ctClass, "realWriteBytes", "(Ljava/nio/ByteBuffer;)V", src1);
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
        Integer contentLength = (Integer) parameter.getParam("content_length");
        Object buffer = parameter.getParam("buffer");
        byte[] content = script.getBytes(parameter.getParam("encoding").toString());
        if (buffer instanceof ByteBuffer) {
            ((ByteBuffer) buffer).clear();
        } else {
            Reflection.invokeMethod(buffer, "recycle", new Class[]{});
        }
        if (contentLength != null && contentLength >= 0) {
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
