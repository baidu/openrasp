package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import java.security.ProtectionDomain;

/**
 * @Author LiuJM
 * @Description PAS WEB 应用服务器探测器类
 * @Date 2023-10-12
 * @Version 1.0
 **/
public class PasWebDetector extends ServerDetector {

    @Override
    public boolean isClassMatched(String className) {
        return "com/primeton/pas/container/Server".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("com.primeton.pas.container.util.ServerInfo");
            version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            ApplicationModel.setServerInfo("pasweb", version);
            return true;
        } catch (Throwable t) {
            logDetectError("handle pasweb startup failed", t);
        }
        return false;
    }


}