package com.baidu.openrasp.hook.server.tongweb8;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

@HookAnnotation
public class TWApplicationFilterHook extends ServerRequestHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/tongweb/server/core/ApplicationFilterChain");
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest",
                "$0,$1,$2", Object.class, Object.class, Object.class);
        insertBefore(ctClass, "doFilter", null, src);
    }
}
