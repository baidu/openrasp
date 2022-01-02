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
public class Tongweb7InputBufferHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/catalina/connector/InputBuffer".equals(className);
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
