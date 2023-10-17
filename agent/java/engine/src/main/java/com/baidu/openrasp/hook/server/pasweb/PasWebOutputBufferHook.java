package com.baidu.openrasp.hook.server.pasweb;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @Author LiuJM
 * @Description //PAS WEB 输出流关闭 hook 点
 * @Date 2023-10-12
 * @Version 1.0
 **/
@HookAnnotation
public class PasWebOutputBufferHook extends ServerOutputCloseHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/primeton/pas/container/connector/OutputBuffer".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
    }

}
