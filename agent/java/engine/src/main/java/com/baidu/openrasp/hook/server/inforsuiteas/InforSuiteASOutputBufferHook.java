package com.baidu.openrasp.hook.server.inforsuiteas;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: InforSuiteAS OutputBuffer Hook
 * @author: codff
 * @create: 2022/03/17
 */
@HookAnnotation
public class InforSuiteASOutputBufferHook extends ServerOutputCloseHook {

        public static String clazzName = null;

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
    }

}
