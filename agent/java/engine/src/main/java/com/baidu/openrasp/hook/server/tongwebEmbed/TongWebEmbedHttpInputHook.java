package com.baidu.openrasp.hook.server.tongwebEmbed;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description:
 * @author: ZouJiaNan
 * @date: 2022/11/2 16:04
 */
@HookAnnotation
public class TongWebEmbedHttpInputHook extends ServerInputHook {
    @Override
    public boolean isClassMatched(String className) {
        return false;
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

    }
}
