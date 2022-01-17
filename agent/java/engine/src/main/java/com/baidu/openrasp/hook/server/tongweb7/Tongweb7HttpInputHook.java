package com.baidu.openrasp.hook.server.tongweb7;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: 插入Tongweb获取请求body处理hook
 * @author: zxl
 * @create: 2022-01-01
 */
@HookAnnotation
public class Tongweb7HttpInputHook extends ServerInputHook {

    private String className;

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        if( "com/tongweb/catalina/connector/InputBuffer".equals(className)
         || "com/tongweb/catalina/connector/CoyoteReader".equals(className)){
            this.className=className;
            return true;
        }
        return false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        if(className.equals("com/tongweb/catalina/connector/InputBuffer")){
            String readByteSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "readByte", "()I", readByteSrc);
            String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
            insertAfter(ctClass, "read", "([BII)I", readSrc);

        }else {
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
