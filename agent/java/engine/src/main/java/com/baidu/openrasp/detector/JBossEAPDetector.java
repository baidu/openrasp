package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.model.ApplicationModel;

import java.lang.reflect.Method;
import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-13.
 */
public class JBossEAPDetector extends ServerDetector {


    @Override
    public boolean isClassMatched(String className) {
        return "org/jboss/modules/Main".equals(className);
    }

    @Override
    public void handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String serverVersion = "";
        try {
            Method getPackageMethod = ClassLoader.class.getDeclaredMethod("getPackage", String.class);
            getPackageMethod.setAccessible(true);
            Package jbossBootPackage = (Package) getPackageMethod.invoke(classLoader, "org.jboss.modules");
            serverVersion = jbossBootPackage.getSpecificationVersion();
        } catch (Throwable t) {
            logDetectError("handle jboss eap startup failed", t);
        } finally {
            ApplicationModel.initServerInfo("jboss eap", serverVersion);
        }
    }

}