package com.baidu.openrasp.hook.server.inforsuite;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: inforsuite output close hook
 * @author: inforsuite
 * @create: 2022/05/20
 */
@HookAnnotation
public class InforSuiteHttpResponseHook extends ServerOutputCloseHook {

	public static String clazzName = null;
    @Override
    public boolean isClassMatched(String className) {
        if ("com/cvicse/inforsuite/grizzly/http/io/OutputBuffer".equals(className)) {
        	clazzName = className;
            return true;
        }
        return false;
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
    }

}
