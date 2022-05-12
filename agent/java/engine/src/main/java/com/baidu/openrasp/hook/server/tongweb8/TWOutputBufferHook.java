package com.baidu.openrasp.hook.server.tongweb8;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

@HookAnnotation
public class TWOutputBufferHook extends ServerOutputCloseHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/server/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
    }
}
