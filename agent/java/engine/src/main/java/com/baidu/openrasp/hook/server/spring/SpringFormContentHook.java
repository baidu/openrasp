package com.baidu.openrasp.hook.server.spring;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.request.HttpServletRequest;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 19-11-12.
 *
 * 针对 Spring 对 PUT/PATCH/DELETE 等请求方法的 x-www-form-urlencoded 参数解析的 hook
 */
@HookAnnotation
public class SpringFormContentHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        // 5.1 之前 HttpPutFormContentRequestWrapper, 5.1 及以后 FormContentRequestWrapper
        return "org/springframework/web/filter/FormContentFilter$FormContentRequestWrapper".equals(className)
                || ("org/springframework/web/filter/" +
                "HttpPutFormContentFilter$HttpPutFormContentRequestWrapper").equals(className);
    }

    @Override
    public String getType() {
        return "spring_form";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(SpringFormContentHook.class, "rebuildRequest", "$0", Object.class);
        insertAfter(ctClass, "<init>", null, src);
    }

    public static void rebuildRequest(Object requestWrapper) {
        if (HookHandler.requestCache.get() != null) {
            String requestId = HookHandler.requestCache.get().getRequestId();
            HookHandler.requestCache.set(new HttpServletRequest(requestWrapper, requestId));
        }

    }

}
