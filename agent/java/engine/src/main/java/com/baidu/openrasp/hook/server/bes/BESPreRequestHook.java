package com.baidu.openrasp.hook.server.bes;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: BES pre-request hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESPreRequestHook extends ServerPreRequestHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/bes/enterprise/webtier/connector/CoyoteAdapter");
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
