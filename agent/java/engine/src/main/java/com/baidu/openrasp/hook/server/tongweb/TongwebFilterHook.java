package com.baidu.openrasp.hook.server.tongweb;

import java.io.IOException;

import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;

/**
 * @description: Tongweb ApplicationFilterChain 处理hook
 * @author: Baimo
 * @create: 2019/06/10
 */
@HookAnnotation
public class TongwebFilterHook extends ServerRequestHook {

	/**
	 * (none-javadoc)
	 *
	 * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
	 */
	@Override
	public boolean isClassMatched(String className) {
		return className.endsWith("com/tongweb/web/thor/core/ApplicationFilterChain");
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
