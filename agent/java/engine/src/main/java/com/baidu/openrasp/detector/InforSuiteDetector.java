/*
 * Copyright 2017-2022 Baidu Inc.
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
 * Created by inforsuite on 22-2-12.
 */
public class InforSuiteDetector extends ServerDetector {

    @Override
    public boolean isClassMatched(String className) {
        return "com/cvicse/loong/enterprise/inforsuite/bootstrap/ASMain".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
//            if (classLoader == null) {
//                classLoader = ClassLoader.getSystemClassLoader();
//            }
            classLoader = Thread.currentThread().getContextClassLoader();
            Class clazz = classLoader.loadClass("com.cvicse.loong.appserv.server.util.Version");
            if (!isJboss(classLoader)) {
                version = (String) Reflection.invokeMethod(null, clazz, "getFullVersion", new Class[]{});
            }
        } catch (Throwable t) {
            logDetectError("handle inforsuite startup failed", t);
        }
        if (!isJboss(classLoader)) {
            if(version != null){
                ApplicationModel.setServerInfo("inforsuite", version);
                return true;
            }
            return false;
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
