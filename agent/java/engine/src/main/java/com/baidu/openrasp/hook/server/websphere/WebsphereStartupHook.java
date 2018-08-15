package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerStartupHook;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @author anyang
 * @Description: 获取websphere的serverInfo
 * @date 2018/8/14 17:08
 */
public class WebsphereStartupHook extends ServerStartupHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/websphere/product/WASProduct".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WebsphereStartupHook.class, "handleWebsphereStartup", "$_");
        insertAfter(ctClass, "getPlatform", null, src);
    }

    public static void handleWebsphereStartup(Object websphere) {
        try {
            String serverVersion = (String) Reflection.invokeMethod(websphere, "getVersion", new Class[]{});
            ApplicationModel.init("websphere",serverVersion);
        } catch (Exception e) {
            HookHandler.LOGGER.warn("handle websphere startup failed", e);
        }

    }
}
