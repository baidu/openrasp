/*
 * Copyright 2017-2020 Baidu Inc.
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
import java.io.FilenameFilter;
import java.io.UnsupportedEncodingException;
import java.lang.instrument.Instrumentation;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
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
        // juli
        try {
            Class clazz = Class.forName("java.nio.file.FileSystems");
            clazz.getMethod("getDefault", new Class[0]).invoke(null);
        } catch (Throwable t) {
            // ignore
        }
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

        if (Module.START_MODE_NORMAL == mode) {
            setStartupOptionForJboss();
        }
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
            return isModularityJdk();
        } catch (Exception e) {
            return false;
        }
    }

    public static boolean isModularityJdk() {
        String javaVersion = System.getProperty("java.version");
        String[] version = javaVersion.split("\\.");
        if (version.length >= 2) {
            int major;
            int minor;
            try {
                major = Integer.parseInt(version[0]);
                minor = Integer.parseInt(version[1]);
            } catch (NumberFormatException e) {
                return false;
            }
            if (major == 1) {
                return minor >= 9;
            } else if (major >= 9) {
                return true;
            }
        } else if (javaVersion.startsWith("9")) {
            return true;
        } else if (javaVersion.length() >= 2) {
            char first = javaVersion.charAt(0);
            char second = javaVersion.charAt(1);
            if (first >= '1' && first <= '9' && second >= '0' && second <= '9') {
                return true;
            }
        }
        return false;
    }

    /**
     * 判断当前进程是否为jboss7 版本，并设置相关属性和预加载包
     */
    public static void setStartupOptionForJboss() {
        String jbossHome = "";
        boolean isJboss = false;
        String splitChar = System.getProperty("path.separator") == null ? ";" : System.getProperty("path.separator");
        String jarPaths[] = System.getProperty("java.class.path").split(splitChar);
        for (int i = 0; i < jarPaths.length; ++i) {
            if (jarPaths[i].endsWith("jboss-modules.jar")) {
                File jarFile = new File(jarPaths[i]);
                if (null != jarFile) {
                    jbossHome = jarFile.getParent();
                }
                isJboss = true;
                break;
            }
        }

        if (isJboss) {
            String moduleBaseDir = "";
            File moduleBase = new File(jbossHome + "/modules/system/layers/base");
            if (null != moduleBase && moduleBase.isDirectory()) {
                moduleBaseDir = jbossHome + "/modules/system/layers/base";
            } else {
                moduleBaseDir = jbossHome + "/modules";
            }
            setSystemProperty(moduleBaseDir);
        }
    }

    /**
     * 设置jboss的jboss.modules.system.pkgs，java.util.logging.manager，以及对logmanager的预加载项
     *
     * @param moduleBaseDir
     */
    public static void setSystemProperty(String moduleBaseDir) {

        String pkgs = System.getProperty("jboss.modules.system.pkgs");
        System.out.println("origin pkgs = " + pkgs);
        if (null == pkgs || !pkgs.contains("baidu.openrasp")) {
            if (pkgs != null && !pkgs.isEmpty()) {
                pkgs = System.setProperty("jboss.modules.system.pkgs",
                        pkgs + ",org.jboss.logmanager,com.baidu.openrasp,com.sdwaf,javax.servlet,javax.el");
            } else {
                pkgs = System.setProperty("jboss.modules.system.pkgs",
                        "org.jboss.logmanager,com.baidu.openrasp,com.sdwaf,javax.servlet,javax.el");
            }
            pkgs = System.setProperty("jboss.modules.system.pkgs", pkgs + ",org.jboss.logmanager,com.baidu.openrasp,com.sdwaf,javax.servlet,javax.el");
            System.out.println("default pkgs = " + pkgs);
        }

        String logManager = System.getProperty("java.util.logging.manager");
        System.out.println("origin java.util.logging.manager = " + logManager);
        if (null == logManager || logManager.isEmpty()) {
            System.setProperty("java.util.logging.manager", "org.jboss.logmanager.LogManager");
            System.out.println("add org.jboss.logmanager.LogManager to java.util.logging.manager");
        } else if (!logManager.contains("org.jboss.logmanager.LogManager")) {
            System.setProperty("java.util.logging.manager", logManager + ",org.jboss.logmanager.LogManager");
            System.out.println("add logmanager on old value=" + logManager);
        }

        String logBootPath = "";
        String splitChar = System.getProperty("path.separator") == null ? ";" : System.getProperty("path.separator");
        logBootPath = appendPathAfterLoadJar(logBootPath, moduleBaseDir + "/org/jboss/logmanager/main/", "jboss-logmanager-");
        logBootPath = appendPathAfterLoadJar(logBootPath, moduleBaseDir + "/org/jboss/log4j/logmanager/main/", "jboss-logmanager-");//wildfly8+
        logBootPath = appendPathAfterLoadJar(logBootPath, moduleBaseDir + "/org/jboss/logmanager/log4j/main/", "jboss-logmanager-");//jboss-as7
        logBootPath = appendPathAfterLoadJar(logBootPath, moduleBaseDir + "/org//apache//log4j/main/", "log4j-");//jboss-as7
        logBootPath = appendPathAfterLoadJar(logBootPath, moduleBaseDir + "/org/wildfly/common/main/", "wildfly-common-");//wildfly16
        String bootClasspath = System.getProperty("sun.boot.class.path");
        if (null == bootClasspath || bootClasspath.isEmpty()) {
            System.setProperty("sun.boot.class.path", logBootPath);
        } else if (false == bootClasspath.contains("jboss-logmanager")) {
            logBootPath = logBootPath + splitChar + bootClasspath;
            System.setProperty("sun.boot.class.path", logBootPath);
            System.out.println("add boot classpath on value=" + logBootPath);
        }

        loadJarFromPath(moduleBaseDir + "/javax/servlet/jsp/api/main/", "jsp-api");
        loadJarFromPath(moduleBaseDir + "/javax/servlet/jstl/api/main/", "jstl-api");
        loadJarFromPath(moduleBaseDir + "/javax/servlet/jstl/api/main/", "taglibs-standard-");//wildfly16
        loadJarFromPath(moduleBaseDir + "/javax/el/api/main/", "el-api");
        loadJarFromPath(moduleBaseDir + "/javax/servlet/api/main/", "servlet-api");
    }

    public static String appendPathAfterLoadJar(String oldPath, String libPath, String jarName) {
        String newPath = oldPath;
        String splitChar = System.getProperty("path.separator") == null ? ";" : System.getProperty("path.separator");
        String jarPath = loadJarFromPath(libPath, jarName);
        if (false == jarPath.isEmpty()) {
            newPath = oldPath.isEmpty() ? jarPath : jarPath + splitChar + oldPath;
        }
        return newPath;
    }

    /*
    *从路径加载文件名称匹配项
     */
    public static String loadJarFromPath(String libPath, String jarName) {
        //boolean bLoaded = false;
        String jarPath = "";
        File libDir = new File(libPath);
        if (false == libDir.isDirectory()) {
            System.out.println("library path:" + libPath + " is not directory.");
            return jarPath;
        }
        File[] jarFiles = libDir.listFiles(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                return (name.endsWith(".jar"));
            }
        });
        for (File file : jarFiles) {
            String filePath = file.getAbsolutePath();
            if (filePath != null) {
                if (filePath.contains(jarName)) {
                    loadJar(file);
                    jarPath = file.getAbsolutePath();
                }
            }
        }
        return jarPath;
    }

    public static boolean loadJar(File file) {
        boolean loadResult = true;
        try {
            Method method = URLClassLoader.class.getDeclaredMethod("addURL", new Class[]{URL.class});
            boolean accessible = method.isAccessible();
            try {
                if (!accessible) {
                    method.setAccessible(true);
                }
                try {
                    URL url = file.toURI().toURL();
                    if (moduleClassLoader instanceof URLClassLoader) {
                        method.invoke(moduleClassLoader, new Object[]{url});
                    } else if (ModuleLoader.isCustomClassloader()) {
                        moduleClassLoader = ClassLoader.getSystemClassLoader();
                        method = moduleClassLoader.getClass().getDeclaredMethod("appendToClassPathForInstrumentation", String.class);
                        method.setAccessible(true);
                        try {
                            method.invoke(moduleClassLoader, file.getCanonicalPath());
                        } catch (Exception e) {
                            method.invoke(moduleClassLoader, file.getAbsolutePath());
                        }
                    }
                } catch (Exception localException) {
                    loadResult = false;
                    localException.printStackTrace();
                }

            } finally {
                method.setAccessible(accessible);
            }
        } catch (NoSuchMethodException e1) {
            loadResult = false;
            e1.printStackTrace();
        } catch (SecurityException e1) {
            loadResult = false;
            e1.printStackTrace();
        }
        //System.out.println("Load jar path:"+file.getAbsolutePath()+", return:"+loadResult);
        return loadResult;
    }

}
