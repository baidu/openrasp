package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 15:32
 */
@HookAnnotation
public class TongWebEmbedRequestEndHook extends ServerRequestEndHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/container/core/ApplicationFilterChain".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
        insertAfter(ctClass, "doFilter", null, requestEndSrc, true);
    }
}
