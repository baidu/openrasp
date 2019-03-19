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

import java.security.ProtectionDomain;

/**
 * @description: undertow 服务器探测
 * @author: anyang
 * @create: 2019/02/22 10:26
 */
public class UndertowDetector extends ServerDetector {
    @Override
    public boolean isClassMatched(String className) {
        return "io/undertow/server/HttpHandler".equals(className);
    }

    @Override
    public boolean handleServerInfo(ClassLoader classLoader, ProtectionDomain domain) {
        String version = "";
        try {
            if (classLoader == null) {
                classLoader = ClassLoader.getSystemClassLoader();
            }
            Class clazz = classLoader.loadClass("io.undertow.Version");
            version = (String) Reflection.invokeMethod(null, clazz, "getVersionString", new Class[]{});
        } catch (Exception e) {
            logDetectError("handle undertow startup failed", e);
        }
        ApplicationModel.setServerInfo("undertow", version);
        return true;
    }
}
