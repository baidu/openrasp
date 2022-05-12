package com.baidu.openrasp.hook.server.tongweb8;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

@HookAnnotation
public class TWHttpInputHook extends ServerRequestHook {
    private String className;

    @Override
    public boolean isClassMatched(String className) {
        if ("com/tongweb/server/connector/InputBuffer".equals(className)
                || "com/tongweb/server/connector/CoyoteReader".equals(className)) {
            this.className = className;
            return true;
        }
        return false;    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        if (className.equals("com/tongweb/server/connector/InputBuffer")) {
            String readByteSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "readByte", "()I", readByteSrc);
            String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
            insertAfter(ctClass, "read", "([BII)I", readSrc);
        } else {
            String src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead", "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "read", "()I", src);
            src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0,$1", int.class, Object.class, char[].class);
            insertAfter(ctClass, "read", "([C)I", src);

            src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0,$1,$2", int.class, Object.class, char[].class, int.class);
            insertAfter(ctClass, "read", "([CII)I", src);
        }
    }
}
