package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 16:09
 */
@HookAnnotation
public class TongWebEmbedApplicationFilterHook extends ServerRequestHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/container/core/ApplicationFilterChain".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest", "$0,$1,$2", Object.class, Object.class,
                Object.class);
        insertBefore(ctClass, "doFilter", null, src);
    }
}
