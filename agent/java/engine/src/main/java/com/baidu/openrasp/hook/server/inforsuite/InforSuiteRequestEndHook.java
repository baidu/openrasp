package com.baidu.openrasp.hook.server.inforsuite;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: inforsuite response end hook
 * @author: inforsuite
 * @create: 2022/05/20
 */
@HookAnnotation
public class InforSuiteRequestEndHook extends ServerRequestEndHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {    	
    	return className.endsWith("org/apache/catalina/core/ApplicationFilterChain");
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
        insertAfter(ctClass, "doFilter", null, requestEndSrc, true);
    }

}
