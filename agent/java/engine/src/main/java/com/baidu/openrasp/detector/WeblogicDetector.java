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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;

import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-12.
 */
public class WeblogicDetector extends ServerDetector {
    @Override
    public boolean isClassMatched(String className) {
        return "weblogic/t3/srvr/T3Srvr".equals(className);
    }

    @Override
    public void handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String serverVersion = "";
        try {
            Class clazz = classLoader.loadClass("weblogic.version");
            serverVersion = (String) Reflection.invokeStaticMethod(clazz.getName(), "getVersions", new Class[]{});
            if (serverVersion != null) {
                int index = getFirstNumIndexFromString(serverVersion);
                if (index >= 0) {
                    serverVersion = serverVersion.substring(index);
                }
            }
        } catch (Throwable t) {
            logDetectError("handle weblogic startup failed", t);
        } finally {
            ApplicationModel.initServerInfo("weblogic", serverVersion);
        }
    }

    private int getFirstNumIndexFromString(String version) {
        for (int i = 0; i < version.length(); i++) {
            if (version.charAt(i) >= '0' && version.charAt(i) <= '9') {
                return i;
            }
        }
        return -1;
    }
}
