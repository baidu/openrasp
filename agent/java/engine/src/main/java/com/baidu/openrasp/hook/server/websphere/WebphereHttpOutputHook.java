package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @author anyang
 * @Description: websphere 响应关闭 hook 点
 * @date 2018/8/13 17:45
 */
@HookAnnotation
public class WebphereHttpOutputHook extends ServerOutputCloseHook{

    public static String clazzName = null;
    @Override
    public boolean isClassMatched(String className) {
        if ("com/ibm/ws/webcontainer/srt/SRTServletResponse".equals(className)){
            clazzName = className;
            return true;
        }
        return false;
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "finish", "()V", src);
    }
}
