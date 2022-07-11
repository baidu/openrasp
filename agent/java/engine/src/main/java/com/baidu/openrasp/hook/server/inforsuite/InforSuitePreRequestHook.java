package com.baidu.openrasp.hook.server.inforsuite;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: inforsuite pre-request hook
 * @author: inforsuite
 * @create: 2022/05/20
 */
@HookAnnotation
public class InforSuitePreRequestHook extends ServerPreRequestHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("org/apache/catalina/connector/CoyoteAdapter");
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
