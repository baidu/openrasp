package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerParamHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @author anyang
 * @Description: websphere参数解析hook点
 * @date 2018/8/13 15:32
 */
@HookAnnotation
public class WebsphereParseParamHook extends ServerParamHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/srt/SRTServletRequest".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "parseParameters", "()V", src);
    }
}
