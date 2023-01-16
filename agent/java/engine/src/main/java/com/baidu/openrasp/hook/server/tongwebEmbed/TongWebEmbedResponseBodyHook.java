package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.hook.server.ServerResponseBodyHook;
import com.baidu.openrasp.hook.server.catalina.CatalinaResponseBodyHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 15:05
 */
@HookAnnotation
public class TongWebEmbedResponseBodyHook extends ServerResponseBodyHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/connector/Response".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(CatalinaResponseBodyHook.class, "getBuffer", "$0,$1", Object.class, Object.class);
        insertBefore(ctClass, "doWrite", "(Lorg/apache/tomcat/util/buf/ByteChunk;)V", src);
        insertBefore(ctClass, "doWrite", "(Ljava/nio/ByteBuffer;)V", src);
    }
}
