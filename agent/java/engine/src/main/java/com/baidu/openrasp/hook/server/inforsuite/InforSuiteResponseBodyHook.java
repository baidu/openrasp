/**
 *
 */
package com.baidu.openrasp.hook.server.inforsuite;

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
import java.nio.CharBuffer;
import java.util.HashMap;

/**
 * @description: inforsuite response body hook
 * @author: inforsuite
 * @create: 2022/05/20
 */
@HookAnnotation
public class InforSuiteResponseBodyHook extends ServerResponseBodyHook {

    @Override
    public boolean isClassMatched(String className) {
    	return "com/cvicse/inforsuite/grizzly/http/io/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src1 = getInvokeStaticSrc(InforSuiteResponseBodyHook.class, "getBufferObject", "$0,$1", Object.class);
        insertBefore(ctClass, "flushCharsToBuf", "(Ljava/nio/CharBuffer;Z)V", src1);
    }

    public static void getBufferObject(Object outputBuffer, CharBuffer charBuf) {
        boolean isCheckXss = isCheckXss();
        boolean isCheckSensitive = isCheckSensitive();
        if (HookHandler.isEnableXssHook() && (isCheckXss || isCheckSensitive) ) {
            HookHandler.disableBodyXssHook();
            HashMap<String, Object> params = new HashMap<String, Object>();
            try {
            	int len = (Integer) Reflection.getSuperField(outputBuffer, "charsArrayLength");
                HttpServletResponse res = HookHandler.responseCache.get();
                String enc = null;
                String contentType = null;
                if (res != null) {
                    enc = res.getCharacterEncoding();
                    contentType = res.getContentType();
                }
                if (enc != null) {
                    params.put("buffer", charBuf);
                    params.put("content_length", len);
                    params.put("encoding", enc);
                    params.put("content_type", contentType);
                    params.put("content",charBuf.toString());
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


    public static void handleXssBlockBuffer(CheckParameter parameter, String script) throws UnsupportedEncodingException {
        Integer contentLength = (Integer) parameter.getParam("content_length");
        Object buffer = parameter.getParam("buffer");
        char[] content = script.toCharArray();
        if (contentLength != null && contentLength >= 0) {
            char[] fullContent = new char[contentLength];
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

    private static void writeContentToBuffer(Object buffer, char[] content) throws UnsupportedEncodingException {
       
        	CharBuffer b = (CharBuffer) buffer;
            if (content.length > b.remaining() && b.remaining() > 0) {
                b.put((char) ' ');
            } else {
                b.put(content, 0, content.length);
            }
            b.flip();

    }

}
