/*
 * Copyright 2017-2018 Baidu Inc.
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
import java.io.UnsupportedEncodingException;
import java.lang.instrument.Instrumentation;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLDecoder;
import java.util.jar.Attributes;
import java.util.jar.JarFile;

/**
 * Created by tyy on 18-1-23.
 *
 * 用于加载和初始化引擎模块
 */
public class ModuleLoader {

    public static final String[] jars = new String[]{"rasp-engine.jar"};

    private static String baseDirectory;

    private static ModuleLoader instance;

    public static ClassLoader moduleClassLoader;


    // ModuleLoader 为 classloader加载的，不能通过getProtectionDomain()的方法获得JAR路径
    static {
        Class clazz = ModuleLoader.class;
        // path值示例：　file:/opt/apache-tomcat-xxx/rasp/rasp.jar!/com/fuxi/javaagent/Agent.class
        String path = clazz.getResource("/" + clazz.getName().replace(".", "/") + ".class").getPath();
        if (path.startsWith("file:")) {
            path = path.substring(5);
        }
        if (path.contains("!")) {
            path = path.substring(0, path.indexOf("!"));
        }
        try {
            baseDirectory = URLDecoder.decode(new File(path).getParent(), "UTF-8");
        } catch (UnsupportedEncodingException e) {
            baseDirectory = new File(path).getParent();
        }
        ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
        while (systemClassLoader.getParent() != null
                && !systemClassLoader.getClass().getName().startsWith("sun.misc.Launcher")) {
            systemClassLoader = systemClassLoader.getParent();
        }
        moduleClassLoader = systemClassLoader;
    }

    /**
     * 构造所有模块
     *
     * @param agentArg premain 传入的命令行参数
     * @param inst     {@link java.lang.instrument.Instrumentation}
     */
    private ModuleLoader(String agentArg, Instrumentation inst) throws Exception {
        for (int i = 0; i < jars.length; i++) {
            Object module = null;
            try {
                File originFile = new File(baseDirectory + File.separator + jars[i]);
                JarFile jarFile = new JarFile(originFile);
                Attributes attributes = jarFile.getManifest().getMainAttributes();
                jarFile.close();
                String moduleName = attributes.getValue("Rasp-Module-Name");
                String moduleEnterClassName = attributes.getValue("Rasp-Module-Class");
                if (moduleName != null && moduleEnterClassName != null
                        && !moduleName.equals("") && !moduleEnterClassName.equals("")) {
                    Method method = Class.forName("java.net.URLClassLoader").getDeclaredMethod("addURL", URL.class);
                    method.setAccessible(true);
                    method.invoke(moduleClassLoader, originFile.toURI().toURL());
                    method.invoke(ClassLoader.getSystemClassLoader(), originFile.toURI().toURL());
                    Class moduleClass = moduleClassLoader.loadClass(moduleEnterClassName);
                    module = moduleClass.newInstance();
                    if (module instanceof Module) {
                        try {
                            moduleClass.getMethod("start", String.class, Instrumentation.class).invoke(module, agentArg, inst);
                        } catch (Exception e) {
                            moduleClass.getMethod("release").invoke(module);
                            throw e;
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
                System.err.println("[OpenRASP] Failed to initialize module jar: " + jars[i]);
                throw e;
            }
        }
    }

    /**
     * 加载所有 RASP 模块
     *
     * @param agentArg premain 传入的命令行参数
     * @param inst     {@link java.lang.instrument.Instrumentation}
     */
    public static synchronized void load(String agentArg, Instrumentation inst) throws Exception {
        if (instance == null) {
            synchronized (ModuleLoader.class) {
                if (instance == null) {
                    instance = new ModuleLoader(agentArg, inst);
                }
            }
        }
    }

}
