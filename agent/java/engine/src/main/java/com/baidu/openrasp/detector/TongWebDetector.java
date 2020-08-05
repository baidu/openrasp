/**
 * 
 */
package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.lang.reflect.Method;
import java.security.ProtectionDomain;

/**
 * @author baimo
 *
 */
public class TongWebDetector extends ServerDetector {

	@Override
	public boolean isClassMatched(String className) {
        return "com/tongweb/web/thor/Server".equals(className);
	}

	@Override
	public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("com.tongweb.web.thor.util.ServerInfo");
            version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            ApplicationModel.setServerInfo("tongweb", version);
            return true;
        } catch (Throwable t) {
            logDetectError("handle Tongweb startup failed", t);
        }
        return false;
    }

}
