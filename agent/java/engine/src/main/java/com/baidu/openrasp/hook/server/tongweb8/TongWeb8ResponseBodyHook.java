package com.baidu.openrasp.hook.server.tongweb8;

import com.baidu.openrasp.hook.server.ServerResponseBodyHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

@HookAnnotation
public class TongWeb8ResponseBodyHook extends ServerResponseBodyHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/coyote/Response".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(TongWeb8ResponseBodyHook.class, "getBuffer", "$0,$1", Object.class, Object.class);
        insertBefore(ctClass, "doWrite", "(Lcom/tongweb/web/util/buf/ByteChunk;)V", src);
        insertBefore(ctClass, "doWrite", "(Ljava/nio/ByteBuffer;)V", src);
    }
}
