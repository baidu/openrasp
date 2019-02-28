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

package com.baidu.openrasp;

import java.io.File;
import java.lang.instrument.Instrumentation;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.Attributes;
import java.util.jar.JarFile;

import static com.baidu.openrasp.ModuleLoader.*;

/**
 * Created by tyy on 19-2-26.
 */
public class ModuleContainer implements Module {

    private Module module;
    private String moduleName;

    public ModuleContainer(String jarName) throws Throwable {
        try {
            File originFile = new File(baseDirectory + File.separator + jarName);
            JarFile jarFile = new JarFile(originFile);
            Attributes attributes = jarFile.getManifest().getMainAttributes();
            jarFile.close();
            this.moduleName = attributes.getValue("Rasp-Module-Name");
            String moduleEnterClassName = attributes.getValue("Rasp-Module-Class");
            if (moduleName != null && moduleEnterClassName != null
                    && !moduleName.equals("") && !moduleEnterClassName.equals("")) {
                Class moduleClass;
                if (ClassLoader.getSystemClassLoader() instanceof URLClassLoader) {
                    Method method = Class.forName("java.net.URLClassLoader").getDeclaredMethod("addURL", URL.class);
                    method.setAccessible(true);
                    method.invoke(moduleClassLoader, originFile.toURI().toURL());
                    method.invoke(ClassLoader.getSystemClassLoader(), originFile.toURI().toURL());
                    moduleClass = moduleClassLoader.loadClass(moduleEnterClassName);
                    module = (Module) moduleClass.newInstance();
                } else if (ModuleLoader.isCustomClassloader()) {
                    moduleClassLoader = ClassLoader.getSystemClassLoader();
                    Method method = moduleClassLoader.getClass().getDeclaredMethod("appendToClassPathForInstrumentation", String.class);
                    method.setAccessible(true);
                    try {
                        method.invoke(moduleClassLoader, originFile.getCanonicalPath());
                    } catch (Exception e) {
                        method.invoke(moduleClassLoader, originFile.getAbsolutePath());
                    }
                    moduleClass = moduleClassLoader.loadClass(moduleEnterClassName);
                    module = (Module) moduleClass.newInstance();
                } else {
                    throw new Exception("[OpenRASP] Failed to initialize module jar: " + jarName);
                }
            }
        } catch (Throwable t) {
            System.err.println("[OpenRASP] Failed to initialize module jar: " + jarName);
            throw t;
        }
    }

    @Override
    public void start(String mode, Instrumentation inst) throws Throwable {
        module.start(mode, inst);
    }

    @Override
    public void release(String mode) throws Throwable {
        try {
            if (module != null) {
                module.release(mode);
            }
        } catch (Throwable t) {
            System.err.println("[OpenRASP] Failed to release module: " + moduleName);
            throw t;
        }
    }

}
