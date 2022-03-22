package com.baidu.openrasp.hook.server.inforsuiteas;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: InforSuiteAS HttpInput Hook
 * @author: codff
 * @create: 2022/03/17
 */
@HookAnnotation
public class InforSuiteASHttpInputHook extends ServerInputHook {

    private String className;

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        if( "org/apache/catalina/connector/InputBuffer".equals(className)
                || "org/apache/catalina/connector/CoyoteReader".equals(className)){
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
        if(className.equals("org/apache/catalina/connector/InputBuffer")){
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
