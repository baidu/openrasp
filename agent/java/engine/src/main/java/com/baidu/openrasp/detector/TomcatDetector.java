/*
 * Copyright 2017-2019 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.baidu.openrasp.detector;

import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.lang.reflect.Method;
import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-12.
 */
public class TomcatDetector extends ServerDetector {

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
            if (!isJboss(classLoader)) {
                version = (String) Reflection.invokeMethod(null, clazz, "getServerNumber", new Class[]{});
            }
        } catch (Throwable t) {
            logDetectError("handle tomcat startup failed", t);
        }
        if (!isJboss(classLoader)) {
            ApplicationModel.setServerInfo("tomcat", version);
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
