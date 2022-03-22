/**
 *
 */
package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.security.ProtectionDomain;

/**
 * @description: InforSuiteAS detector
 * @author: codff
 * @create: 2022/3/17
 */
public class InforSuiteASDetector extends ServerDetector {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/Server".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("org.apache.catalina.util.ServerInfo");
            version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            ApplicationModel.setServerInfo("inforsuiteas", version);
            return true;
        } catch (Throwable t) {
            logDetectError("handle inforsuiteas startup failed", t);
        }
        return false;
    }

}
