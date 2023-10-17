package com.baidu.openrasp.hook.server.pasweb;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @Author LiuJM
 * @Description PAS WEB servlet过滤器hook类
 * @Date 2023-10-12
 * @Version 1.0
 **/
@HookAnnotation
public class PasWebApplicationFilterHook extends ServerRequestHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/primeton/pas/container/core/ApplicationFilterChain");
    }
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest",
                "$0,$1,$2", Object.class, Object.class, Object.class);
        insertBefore(ctClass, "doFilter", null, src);
    }

}
