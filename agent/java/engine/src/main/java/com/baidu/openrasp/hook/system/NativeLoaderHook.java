package com.baidu.openrasp.hook.system;

import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 19-7-23.
 */
@HookAnnotation
public class NativeLoaderHook extends LoadLibraryHook {

    @Override
    public boolean isClassMatched(String className) {
        return "java/lang/ClassLoader$NativeLibrary".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(LoadLibraryHook.class,
                "checkSystemLoadPath", "$2", String.class);
        insertBefore(ctClass, "<init>", null, src);
    }
}
