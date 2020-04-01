package com.baidu.openrasp.hook.server.bes;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: BES ApplicationFilterChain hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESwebFilterHook extends ServerRequestHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/bes/enterprise/webtier/core/ApplicationFilterChain");
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest", "$0,$1,$2", Object.class, Object.class,
                Object.class);
        insertBefore(ctClass, "doFilter", null, src);
    }

}
