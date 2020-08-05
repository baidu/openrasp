package com.baidu.openrasp.hook.system;

import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.File;
import java.io.IOException;

/**
 * Created by tyy on 19-7-23.
 */
@HookAnnotation
public class ClassLoaderHook extends LoadLibraryHook {

    @Override
    public boolean isClassMatched(String className) {
        return "java/lang/ClassLoader".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String beforeSrc = getInvokeStaticSrc(LoadLibraryHook.class,
                "checkSystemLoadFile", "$2", File.class);
        String afterSrc = getInvokeStaticSrc(LoadLibraryHook.class,
                "closeCheckSystemLoad", "");
        insertBefore(ctClass, "loadLibrary0", "(Ljava/lang/Class;Ljava/io/File;)Z", beforeSrc);
        insertAfter(ctClass, "loadLibrary0", "(Ljava/lang/Class;Ljava/io/File;)Z", afterSrc, true);
    }

}
