package com.baidu.openrasp.hook.server.tongweb;

import java.io.IOException;

import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;

/**
 * @description: 插入Tongweb获取请求body处理hook
 * @author: Baimo
 * @create: 2019/06/11
 */
@HookAnnotation
public class TongwebInputBufferHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/web/thor/connector/InputBuffer".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "readByte", "()I", readByteSrc);        
        String readSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0,$1,$2,$3", int.class, Object.class, byte[].class, int.class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);       
    }
}
