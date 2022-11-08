package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 16:07
 */
@HookAnnotation
public class TongWebEmbedCoyoteAdapterHook extends ServerPreRequestHook {
    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "service", null, src);
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/container/connector/CoyoteAdapter".equals(className);
    }
}
