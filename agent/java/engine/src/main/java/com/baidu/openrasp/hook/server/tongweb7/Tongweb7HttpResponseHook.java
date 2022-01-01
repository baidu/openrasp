package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: 插入Tongweb服务器应答处理hook
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7HttpResponseHook extends ServerOutputCloseHook {

    public static String clazzName = null;

	@Override
	public boolean isClassMatched(String className) {
        if ("com/tongweb/catalina/connector/Response".equals(className)) {
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
