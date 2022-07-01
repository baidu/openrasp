package com.baidu.openrasp.hook.server.inforsuite;

/**
 * @description: inforsuite request hook
 * @author: inforsuite
 * @create: 2022/05/20
 */

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

@HookAnnotation
public class InforSuiteRequestHook extends ServerParamHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/Request".equals(className);
	
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertAfter(ctClass, "processParameters", null, src);
    }

}
