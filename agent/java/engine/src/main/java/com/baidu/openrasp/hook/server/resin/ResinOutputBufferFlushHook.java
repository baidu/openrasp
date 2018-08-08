package com.baidu.openrasp.hook.server.resin;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.hook.ServerXss;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;


/**
　　* @Description: resin 获取输出buffer的hook点
　　* @author anyang
　　* @date 2018/8/7 19:27
　　*/
public class ResinOutputBufferFlushHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/http/ToByteResponseStream".equals(className) ||
                "com/caucho/server/connection/ToByteResponseStream".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(ResinOutputBufferFlushHook.class, "getResinOutputBuffer", "_charBuffer,_charLength,_isOutputStreamOnly", char[].class, int.class, boolean.class);
        insertBefore(ctClass, "flushCharBuffer", "()V", src);

    }

    public static void getResinOutputBuffer(char[] buffer, int len, boolean isOutputStreamOnly) {

        if (len > 0 && !isOutputStreamOnly) {
            char[] temp = new char[len];
            System.arraycopy(buffer, 0, temp, 0, len);
            String content = new String(temp);
            try {
                HashMap<String, Object> params = ServerXss.generateXssParameters(content);
                HookHandler.doCheck(CheckParameter.Type.XSS, params);

            } catch (Exception e) {

                e.printStackTrace();
            }
        }

    }
}
