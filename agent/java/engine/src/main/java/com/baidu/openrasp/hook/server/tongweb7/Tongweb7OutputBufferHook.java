package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: 输出流关闭 hook 点
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7OutputBufferHook extends ServerOutputCloseHook {

	@Override
	public boolean isClassMatched(String className) {
        return "com/tongweb/catalina/connector/OutputBuffer".equals(className);
    }
    
	@Override
	protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "close", "()V", src);
	}

}
