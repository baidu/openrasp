package com.baidu.openrasp.hook.server.bes;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: BES output close hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESHttpResponseHook extends ServerOutputCloseHook {

    public static String clazzName = null;

    @Override
    public boolean isClassMatched(String className) {
        if ("com/bes/enterprise/webtier/connector/Response".equals(className)) {
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
