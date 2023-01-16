package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 16:02
 */
@HookAnnotation
public class TongwebEmbedOutputBufferHook extends ServerOutputCloseHook {
    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/container/connector/OutputBuffer".equals(className);
    }
}
