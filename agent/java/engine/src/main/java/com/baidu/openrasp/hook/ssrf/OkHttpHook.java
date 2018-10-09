package com.baidu.openrasp.hook.ssrf;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @program openrasp
 * @description: okhttp的ssrf检测hook点
 * @author: anyang
 * @create: 2018/10/09 19:40
 */
@HookAnnotation
public class OkHttpHook extends AbstractSSRFHook{
    @Override
    public boolean isClassMatched(String className) {
        return "okhttp3/HttpUrl".equals(className)||
                "com/squareup/okhttp/HttpUrl".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(OkHttpHook.class, "checkOkHttpUrl",
                "$1,$_", String.class,Object.class);
        insertAfter(ctClass, "parse", "(Ljava/lang/String;)Lokhttp3/HttpUrl;", src);
    }

    public static void checkOkHttpUrl(String url, Object httpUrl){
        String host = null;
        if (httpUrl!=null){
            host = Reflection.invokeStringMethod(httpUrl, "host", new Class[]{});
        }
        if (host != null) {
            checkHttpUrl(url, host, "okhttp");
        }
    }
}
