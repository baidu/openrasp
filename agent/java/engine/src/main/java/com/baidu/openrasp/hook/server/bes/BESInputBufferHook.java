package com.baidu.openrasp.hook.server.bes;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: BES input buffer hook
 * @author: bes
 * @create: 2020/03/20
 */
@HookAnnotation
public class BESInputBufferHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/bes/enterprise/webtier/connector/InputBuffer".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "readByte", "()I", readByteSrc);        
        String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);       
    }
}
