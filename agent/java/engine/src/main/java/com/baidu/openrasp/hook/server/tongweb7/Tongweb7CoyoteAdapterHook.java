package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: Tongweb ServerPreRequestHook 处理hook，会执行onParseParameters 进行预处理
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7CoyoteAdapterHook extends ServerPreRequestHook {

    public Tongweb7CoyoteAdapterHook() {
        couldIgnore=false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return className.endsWith("com/tongweb/catalina/connector/CoyoteAdapter");
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
