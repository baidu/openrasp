package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @Description: websphere请求hook点
 * @author anyang
 * @date 2018/8/13 15:13
 */
@HookAnnotation
public class WebsphereRequestHook extends ServerRequestHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/servlet/ServletWrapper".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String hookDesc = "(Ljavax/servlet/ServletRequest;Ljavax/servlet/ServletResponse;)V";
        String src = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest",
                "$0,$1,$2", Object.class, Object.class, Object.class);
        insertBefore(ctClass, "handleRequest", hookDesc, src);
    }
}
