/**
 *
 */
package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.security.ProtectionDomain;

/**
 * @description: BES detector
 * @author: bes
 * @create: 2020/03/20
 */
public class BESDetector extends ServerDetector {

    @Override
    public boolean isClassMatched(String className) {
        return "com/bes/enterprise/webtier/Server".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("com.bes.enterprise.webtier.util.ServerInfo");
            version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            ApplicationModel.setServerInfo("bes", version);
            return true;
        } catch (Throwable t) {
            logDetectError("handle Tongweb startup failed", t);
        }
        return false;
    }

}
