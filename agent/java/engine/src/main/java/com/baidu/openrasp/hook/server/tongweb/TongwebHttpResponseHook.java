package com.baidu.openrasp.hook.server.tongweb;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: 插入Tongweb服务器应答处理hook
 * @author: Baimo
 * @create: 2019/06/10
 */
@HookAnnotation
public class TongwebHttpResponseHook extends ServerOutputCloseHook {

    public static String clazzName = null;

    @Override
    public boolean isClassMatched(String className) {
        if ("com/tongweb/web/thor/connector/Response".equals(className)) {
            clazzName = className;
            return true;
        }
        return false;
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "finishResponse", "()V", src);
    }

}
