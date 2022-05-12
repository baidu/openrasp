package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.lang.reflect.Method;
import java.security.ProtectionDomain;

/**
 * tongweb8
 */
public class TWDetector extends ServerDetector {
    @Override
    public boolean isClassMatched(String className) {
        return "com/tongweb/server/Server".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("com.tongweb.server.startup.ServerInfo");
            if (!isJboss(classLoader)) {  //  TODO 是否需要判断jboss
                version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            }
        } catch (Throwable t) {
            logDetectError("handle TW startup failed", t);
        }
        if (!isJboss(classLoader)) {
            ApplicationModel.setServerInfo("TW", version);
            return true;
        }
        return false;
    }

    private boolean isJboss(ClassLoader classLoader) {
        Package jbossBootPackage = null;
        try {
            Method getPackageMethod = ClassLoader.class.getDeclaredMethod("getPackage", String.class);
            getPackageMethod.setAccessible(true);
            jbossBootPackage = (Package) getPackageMethod.invoke(classLoader, "org.jboss");
            if (jbossBootPackage == null) {
                jbossBootPackage = (Package) getPackageMethod.invoke(classLoader, "org.jboss.modules");
            }
        } catch (Throwable e) {
            // ignore
        }
        return jbossBootPackage != null;
    }
}
