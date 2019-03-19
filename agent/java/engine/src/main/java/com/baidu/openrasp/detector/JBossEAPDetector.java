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
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        if (isWildfly(domain)){
            return false;
        }
        String serverVersion = "";
        try {
            Method getPackageMethod = ClassLoader.class.getDeclaredMethod("getPackage", String.class);
            getPackageMethod.setAccessible(true);
            Package jbossBootPackage = (Package) getPackageMethod.invoke(classLoader, "org.jboss.modules");
            serverVersion = jbossBootPackage.getSpecificationVersion();
        } catch (Throwable t) {
            logDetectError("handle jboss eap startup failed", t);
        }
        ApplicationModel.setServerInfo("jboss eap", serverVersion);
        return true;
    }

}