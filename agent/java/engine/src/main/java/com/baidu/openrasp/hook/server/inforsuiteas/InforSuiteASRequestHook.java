package com.baidu.openrasp.hook.server.inforsuiteas;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: InforSuiteAS Request Hook
 * @author: codff
 * @create: 2022/03/17
 */
@HookAnnotation
public class InforSuiteASRequestHook extends ServerParamHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/Request".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertAfter(ctClass, "parseParameters", "()V", src);
    }

}
