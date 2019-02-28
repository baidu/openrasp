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
import java.io.UnsupportedEncodingException;
import java.lang.instrument.Instrumentation;
import java.net.URLDecoder;

/**
 * Created by tyy on 18-1-23.
 *
 * 用于加载和初始化引擎模块
 */
public class ModuleLoader {

    public static final String ENGINE_JAR = "rasp-engine.jar";

    private static ModuleContainer engineContainer;

    public static String baseDirectory;

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
                && !systemClassLoader.getClass().getName().equals("sun.misc.Launcher$ExtClassLoader")) {
            systemClassLoader = systemClassLoader.getParent();
        }
        moduleClassLoader = systemClassLoader;
    }

    /**
     * 构造所有模块
     *
     * @param mode 启动模式
     * @param inst {@link java.lang.instrument.Instrumentation}
     */
    private ModuleLoader(String mode, Instrumentation inst) throws Throwable {
        engineContainer = new ModuleContainer(ENGINE_JAR);
        engineContainer.start(mode, inst);
    }

    public static synchronized void release(String mode) {
        try {
            if (engineContainer != null) {
                System.out.println("[OpenRASP] Start to release OpenRASP");

                engineContainer.release(mode);

                engineContainer = null;
            } else {
                System.out.println("[OpenRASP] The OpenRASP has not be bean initialized");
            }
        } catch (Throwable throwable) {
            // ignore
        }
    }

    /**
     * 加载所有 RASP 模块
     *
     * @param mode 启动模式
     * @param inst {@link java.lang.instrument.Instrumentation}
     */
    public static synchronized void load(String mode, String action, Instrumentation inst) throws Throwable {
        if (Module.START_ACTION_INSTALL.equals(action)) {
            if (instance == null) {
                try {
                    instance = new ModuleLoader(mode, inst);
                } catch (Throwable t) {
                    instance = null;
                    throw t;
                }
            } else {
                System.out.println("[OpenRASP] The OpenRASP has bean initialized and cannot be initialized again");
            }
        } else if (Module.START_ACTION_UNINSTALL.equals(action)) {
            release(mode);
        } else {
            throw new IllegalStateException("[OpenRASP] Can not support the action: " + action);
        }
    }


    /**
     * 判断是否是weblogic或者jdk9、10和11
     */
    public static boolean isCustomClassloader() {
        try {
            String classLoader = ClassLoader.getSystemClassLoader().getClass().getName();
            if (classLoader.startsWith("com.oracle") && classLoader.contains("weblogic")) {
                return true;
            }
            String javaVersion = System.getProperty("java.version");
            return javaVersion != null && (javaVersion.startsWith("1.9") || javaVersion.startsWith("10.")
                    || javaVersion.startsWith("11."));
        } catch (Exception e) {
            return false;
        }
    }

}
