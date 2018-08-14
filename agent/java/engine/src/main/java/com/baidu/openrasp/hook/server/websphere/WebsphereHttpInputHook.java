package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerInputHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @author anyang
 * @Description: 获取websphere的body
 * @date 2018/8/13 17:18
 */
public class WebsphereHttpInputHook extends ServerInputHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/srt/http/HttpInputStream".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "read", "()I", readByteSrc);
        String readSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0,$1,$2,$3", int.class, Object.class, byte[].class, int.class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);
    }
}
