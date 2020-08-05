package com.baidu.openrasp.hook.ssrf.redirect;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.net.URL;

/**
 * Created by tyy on 19-11-27.
 */
@HookAnnotation
public class URLConnectionRedirectHook extends AbstractRedirectHook {

    public static ThreadLocal<URL> urlCache = new ThreadLocal<URL>() {
        @Override
        protected URL initialValue() {
            return null;
        }
    };

    @Override
    public boolean isClassMatched(String className) {
        return "sun/net/www/protocol/http/HttpURLConnection".equals(className) ||
                "weblogic/net/http/HttpURLConnection".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(URLConnectionRedirectHook.class, "cacheHttpRedirect",
                "$0,($w)$_", Object.class, Object.class);
        insertAfter(ctClass, "followRedirect", "()Z", src, false);
    }

    public static void cacheHttpRedirect(Object connection, Object isRedirect) {
        if ((Boolean) isRedirect) {
            try {
                urlCache.set((URL) Reflection.invokeMethod(connection, "getURL", new Class[0]));
            } catch (Throwable t) {
                // ignore
            }
        }
    }
}
