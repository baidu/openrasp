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

import java.security.ProtectionDomain;

/**
 * Created by tyy on 19-2-12.
 */
public class ResinDetector extends ServerDetector {

    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/resin/Resin".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String serverVersion = "";
        try {
            Class versionClass = classLoader.loadClass("com.caucho.Version");
            serverVersion = (String) versionClass.getField("VERSION").get(null);
        } catch (Throwable t) {
            logDetectError("handle jetty startup failed", t);
        }
        ApplicationModel.setServerInfo("resin", serverVersion);
        return true;
    }
}
