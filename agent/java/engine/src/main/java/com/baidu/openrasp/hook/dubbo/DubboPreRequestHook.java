package com.baidu.openrasp.hook.dubbo;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
　　* @Description: 在进入dubbo的hook之前，关掉当前的hook点
　　* @author anyang
　　* @date 2018/6/22 17:30
　　*/
public class DubboPreRequestHook extends AbstractClassHook {

    public DubboPreRequestHook() {

        couldIgnore=false;
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/alibaba/dubbo/rpc/protocol/ProtocolFilterWrapper".equals(className);
    }

    @Override
    public String getType() {
        return "dubbo_preRequest";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(HookHandler.class, "onDubboExit", "");
        insertBefore(ctClass, "export", null, src);

    }
}
