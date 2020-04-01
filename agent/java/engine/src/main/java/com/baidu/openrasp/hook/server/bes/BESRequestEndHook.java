package com.baidu.openrasp.hook.server.bes;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: BES response end hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESRequestEndHook extends ServerRequestEndHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/bes/enterprise/webtier/core/ApplicationFilterChain");
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
        insertAfter(ctClass, "doFilter", null, requestEndSrc, true);
    }

}
