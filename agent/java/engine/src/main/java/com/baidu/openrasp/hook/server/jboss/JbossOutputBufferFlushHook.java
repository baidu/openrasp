package com.baidu.openrasp.hook.server.jboss;

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
import java.util.Map;

/**
　　* @Description: JBoss 获取输出buffer的hook点
　　* @author anyang
　　* @date 2018/8/7 19:29
　　*/
public class JbossOutputBufferFlushHook extends AbstractClassHook {
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

        String src=getInvokeStaticSrc(JbossOutputBufferFlushHook.class,"getJbossOutputBuffer","$0", Object.class);
        insertBefore(ctClass, "close", "()V", src);

    }

    public static void getJbossOutputBuffer(Object object){

            try {
                Object buffer= Reflection.getField(object,"bb");
                String content=new String(buffer.toString().getBytes(),"utf-8");
                if (content.contains("<html>")&&!content.contains("Content-Length")){
                    Map<String,String> serverInfo= HookHandler.requestCache.get().getServerContext();
                    if (serverInfo.get("server").contains("JBoss")){
                        HashMap<String, Object> params = ServerXss.generateXssParameters(content);
                        HookHandler.doCheck(CheckParameter.Type.XSS, params);
                    }
                }

            } catch (Exception e) {

                e.printStackTrace();
            }


    }

}
