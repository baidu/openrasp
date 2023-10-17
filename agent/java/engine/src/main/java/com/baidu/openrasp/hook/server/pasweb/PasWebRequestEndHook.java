package com.baidu.openrasp.hook.server.pasweb;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @Author LiuJM
 * @Description //PAS WEB requestEnd hook点
 * @Date 2023-10-12
 * @Version 1.0
 **/
@HookAnnotation
public class PasWebRequestEndHook extends ServerRequestEndHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/primeton/pas/container/core/ApplicationFilterChain");
    }
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
        insertAfter(ctClass, "doFilter", null, requestEndSrc, true);
    }
}
