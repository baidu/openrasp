package com.baidu.openrasp.hook.server.tongweb;

import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;

/**
 * @description: 插入Tongweb获取request请求处理hook
 * @author: Baimo
 * @create: 2019/06/11
 */
@HookAnnotation
public class TongwebRequestHook extends ServerParamHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/web/thor/connector/Request".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "parseParameters", "()V", src);
    }

}
