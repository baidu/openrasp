package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: Tongweb ApplicationFilterChain 处理hook
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7ApplicationFilterHook extends ServerRequestHook {

	/**
	 * (none-javadoc)
	 *
	 * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
	 */
	@Override
	public boolean isClassMatched(String className) {
		return className.endsWith("com/tongweb/catalina/core/ApplicationFilterChain");
	}

	/**
	 * (none-javadoc)
	 *
	 * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
	 */
	@Override
	protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
		String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest", "$0,$1,$2", Object.class, Object.class,
				Object.class);
		insertBefore(ctClass, "doFilter", null, src);
	}

}
