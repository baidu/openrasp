package com.baidu.openrasp.hook.server.tongweb;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: Tongweb ServerPreRequestHook 处理hook，会执行onParseParameters 进行预处理
 * @author: Baimo
 * @create: 2019/06/18
 */
@HookAnnotation
public class TongwebPreRequestHook extends ServerPreRequestHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/tongweb/web/thor/connector/CoyoteAdapter");
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.server.ServerPreRequestHook#hookMethod(CtClass, String)
     */
    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "service", null, src);
    }

}
