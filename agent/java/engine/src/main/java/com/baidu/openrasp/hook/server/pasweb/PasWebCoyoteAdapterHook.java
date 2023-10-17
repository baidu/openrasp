package com.baidu.openrasp.hook.server.pasweb;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @Author LiuJM
 * @Description PAS WEB
 * @Date 2023-10-12
 * @Version 1.0
 **/
@HookAnnotation
public class PasWebCoyoteAdapterHook extends ServerPreRequestHook {

    public PasWebCoyoteAdapterHook() {
        couldIgnore = false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/primeton/pas/container/connector/CoyoteAdapter");
    }
    /**
     * (none-javadoc)
     *
     * @see ServerPreRequestHook#hookMethod(CtClass, String)
     */
    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "service", null, src);
    }

}
