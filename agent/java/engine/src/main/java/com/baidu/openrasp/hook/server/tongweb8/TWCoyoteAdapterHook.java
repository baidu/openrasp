package com.baidu.openrasp.hook.server.tongweb8;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

@HookAnnotation
public class TWCoyoteAdapterHook extends ServerPreRequestHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/tongweb/server/connector/CoyoteAdapter");
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "service", null, src);

    }
}
