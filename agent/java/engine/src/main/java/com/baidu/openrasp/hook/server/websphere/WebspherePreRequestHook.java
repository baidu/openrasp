package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerPreRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;


/**
 * @Description: websphere请求预处理hook点
 * @author anyang
 * @date 2018/8/13 15:13
 */
@HookAnnotation
public class WebspherePreRequestHook extends ServerPreRequestHook {

    public WebspherePreRequestHook() {
        couldIgnore = false;
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/servlet/ServletWrapperImpl".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        String hookDesc = "(Ljavax/servlet/ServletRequest;Ljavax/servlet/ServletResponse;)V";
        insertBefore(ctClass, "handleRequest", hookDesc, src);
    }
}
