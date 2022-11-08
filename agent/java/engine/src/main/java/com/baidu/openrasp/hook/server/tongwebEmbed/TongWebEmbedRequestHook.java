package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 14:53
 */
@HookAnnotation
public class TongWebEmbedRequestHook extends ServerParamHook {
    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertAfter(ctClass, "parseParameters", "()V", src);
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/container/connector/Request".equals(className);
    }
}
