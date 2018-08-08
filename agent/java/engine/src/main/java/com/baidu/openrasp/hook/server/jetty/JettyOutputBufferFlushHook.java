package com.baidu.openrasp.hook.server.jetty;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.hook.ServerXss;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;

/**
　　* @Description: jetty 获取输出buffer的hook点
　　* @author anyang
　　* @date 2018/8/7 19:25
　　*/
public class JettyOutputBufferFlushHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/eclipse/jetty/server/AbstractHttpConnection".equals(className);
    }

    @Override
    public String getType() {
        return "xss";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(JettyOutputBufferFlushHook.class, "getJettyOutputBuffer", "_generator", Object.class);
        insertBefore(ctClass, "completeResponse", "()V", src);

    }


    public static void getJettyOutputBuffer(Object object) {

        boolean isEnableXssHook = HookHandler.isEnableXssHook();
        if (isEnableXssHook) {
            HookHandler.disableXssHook();

            try {
                Object buffer = Reflection.getSuperField(object, "_buffer");
                String content = new String(buffer.toString().getBytes(), "utf-8");
                HashMap<String, Object> params = ServerXss.generateXssParameters(content);
                HookHandler.doCheck(CheckParameter.Type.XSS, params);

            } catch (Exception e) {

                e.printStackTrace();
            }

        }

    }
}
