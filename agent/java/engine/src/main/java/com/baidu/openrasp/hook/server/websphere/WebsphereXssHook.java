package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @author anyang
 * @Description: websphere的xss检测hook点
 * @date 2018/8/15 14:18
 */
@HookAnnotation
public class WebsphereXssHook extends ServerXssHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/wsspi/webcontainer/util/BufferedWriter".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(WebsphereXssHook.class, "getWebsphereOutputBuffer", "$0", Object.class);
        insertBefore(ctClass, "flushChars", "()V", src);
    }

    public static void getWebsphereOutputBuffer(Object object){
        try {
            char[] buffer = (char[]) Reflection.getField(object,"buf");
            int len = (Integer) Reflection.getField(object,"count");
            char[] temp = new char[len];
            System.arraycopy(buffer,0,temp,0,len);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
