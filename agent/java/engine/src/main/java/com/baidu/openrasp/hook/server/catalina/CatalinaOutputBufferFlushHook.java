package com.baidu.openrasp.hook.server.catalina;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.request.HttpServletRequest;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.hook.ServerXss;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.*;

/**
 * 　　* @Description: catalina 获取输出buffer的hook点
 * 　　* @author anyang
 * 　　* @date 2018/6/11 14:53
 */
public class CatalinaOutputBufferFlushHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/OutputBuffer".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(CatalinaOutputBufferFlushHook.class, "getJbossOutputBuffer", "$0", Object.class);
        insertBefore(ctClass, "close", "()V", src);

    }

    public static void getJbossOutputBuffer(Object object) {
        try {
            String serverType = getServerType();
            String content = null;
            if ("tomcat".equalsIgnoreCase(serverType)) {

                Object charChunk = Reflection.getField(object, "cb");
                char[] buffer = (char[]) Reflection.getField(charChunk, "buff");
                int start = (Integer) Reflection.getSuperField(charChunk, "start");
                int end = (Integer) Reflection.getSuperField(charChunk, "end");
                if (end > start) {
                    char[] temp = new char[end - start + 1];
                    System.arraycopy(buffer, start, temp, 0, end - start);
                    content = new String(temp);
                }


            } else if ("jboss".equalsIgnoreCase(serverType)) {
                Object byteChunk = Reflection.getField(object, "bb");
                byte[] buffer = (byte[]) Reflection.getField(byteChunk, "buff");
                int start = (Integer) Reflection.getField(byteChunk, "start");
                int end = (Integer) Reflection.getField(byteChunk, "end");
                if (end > start) {
                    byte[] temp = new byte[end - start + 1];
                    System.arraycopy(buffer, start, temp, 0, end - start);
                    content = new String(temp, "utf-8");
                }

            }

            if (content != null) {

                HashMap<String, Object> params = ServerXss.generateXssParameters(content);
                HookHandler.doCheck(CheckParameter.Type.XSS, params);

            }

        } catch (Exception e) {

            e.printStackTrace();
        }


    }

    public static String getServerType() {
        String serverInfo = (String) Reflection.invokeStaticMethod("org.apache.catalina.util.ServerInfo",
                "getServerInfo", new Class[]{});
        return HttpServletRequest.extractType(serverInfo);
    }
}
