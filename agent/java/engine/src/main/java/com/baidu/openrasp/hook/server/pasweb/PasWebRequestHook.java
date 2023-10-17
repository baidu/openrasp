package com.baidu.openrasp.hook.server.pasweb;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @Author LiuJM
 * @Description PAS WEB request hook 点
 * @Date 2023-10-12
 * @Version 1.0
 **/
@HookAnnotation
public class PasWebRequestHook extends ServerParamHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/primeton/pas/container/connector/Request".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertAfter(ctClass, "parseParameters", "()V", src);
    }

}
