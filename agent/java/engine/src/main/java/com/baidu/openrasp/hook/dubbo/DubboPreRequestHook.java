package com.baidu.openrasp.hook.dubbo;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @author anyang
 * @Description: dubbo请求预处理hook点
 * @date 2018/8/1315:13
 */
public class DubboPreRequestHook extends AbstractClassHook {

    public DubboPreRequestHook() {

        couldIgnore=false;
    }

    @Override
    public boolean isClassMatched(String className) {
        return "com/alibaba/dubbo/rpc/filter/GenericFilter".equals(className);
    }

    @Override
    public String getType() {
        return "dubbo_preRequest";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {

        String src = getInvokeStaticSrc(HookHandler.class, "onDubboExit", "");
        insertBefore(ctClass, "invoke", null, src);

    }
}
