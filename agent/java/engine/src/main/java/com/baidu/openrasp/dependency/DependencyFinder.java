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

package com.baidu.openrasp.dependency;

import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.io.*;
import java.security.ProtectionDomain;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Properties;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;

/**
 * @description: 收集服务器部署项目的依赖信息
 * @author: anyang
 * @create: 2019/04/19 14:15
 */
public class DependencyFinder {
    public static final Logger LOGGER = Logger.getLogger(DependencyFinder.class.getPackage().getName() + ".log");
    private static final int MAX_DEPENDENCES_CACHE = 10000;
    private static final String DEPENDENCY_SOURCE_MANEFEST_IMPL = "manifest_implementation";
    private static final String DEPENDENCY_SOURCE_MANEFEST_SPEC = "manifest_specification";
    private static final String DEPENDENCY_SOURCE_MANEFEST_BUNDLE = "manifest_bundle";
    private static final String DEPENDENCY_SOURCE_POM = "pom";

    public static ConcurrentSkipListSet<String> loadedJarPaths = new ConcurrentSkipListSet<String>();

    public static void addJarPath(ProtectionDomain domain) {
        if (domain != null && domain.getCodeSource() != null && domain.getCodeSource().getLocation() != null) {
            String path = domain.getCodeSource().getLocation().getFile();
            if (!StringUtils.isEmpty(path)) {
                if ((path.endsWith(".jar")
                        || path.endsWith(".jar!")
                        || path.endsWith(".jar!/")
                        || path.endsWith(".jar/")
                        || path.endsWith(".jar!" + File.separator))
                        && !(loadedJarPaths.size() >= MAX_DEPENDENCES_CACHE)) {
                    if (!path.endsWith(".jar")) {
                        int start = path.contains("/") ? path.indexOf("/") : path.indexOf("\\");
                        path = path.substring(start, path.lastIndexOf(".jar") + 4);
                    }
                    loadedJarPaths.add(path);
                }
            }
        }
    }

    public static HashSet<Dependency> getDependencySet() {
        HashSet<Dependency> dependencySet = new HashSet<Dependency>();
        for (String path : loadedJarPaths) {
            String realPath = path;
            String subPath = null;
            int step = 6;
            int i = path.indexOf(".jar!");
            if (i < 0) {
                step = 5;
                i = path.indexOf(".jar/");
            }
            if (i > 0) {
                realPath = path.substring(0, i + 4);
                subPath = path.substring(i + step, path.length());
            }
            JarFile jarFile;
            try {
                jarFile = new JarFile(realPath);
            } catch (IOException e) {
                if (e instanceof FileNotFoundException) {
                    loadedJarPaths.remove(path);
                } else {
                    LogTool.traceWarn(ErrorType.DEPENDENCY_ERROR,
                            "failed to create jar file from " + path + ": " + e.getMessage(), e);
                }
                continue;
            }
            try {
                Dependency dependency = loadDependencyFromJarFile(jarFile, path);
                if (dependency != null) {
                    dependencySet.add(dependency);
                }
                if (subPath != null) {
                    dependency = loadDependencyFromJar(jarFile, path, subPath);
                    if (dependency != null) {
                        dependencySet.add(dependency);
                    }
                }
            } catch (Exception e) {
                LogTool.traceWarn(ErrorType.DEPENDENCY_ERROR,
                        "failed to parse dependency from jar file " + path + ": " + e.getMessage(), e);
            }
            try {
                jarFile.close();
            } catch (IOException e) {
                LogTool.traceWarn(ErrorType.DEPENDENCY_ERROR,
                        "failed to close jar file " + path + ": " + e.getMessage(), e);
            }
        }

        return dependencySet;
    }

    private static Dependency loadDependencyFromJarFile(JarFile jarFile, String path) throws Exception {
        Dependency dependency = loadDependencyFromPOM(jarFile, path);
        if (dependency != null) {
            return dependency;
        } else {
            dependency = loadDependencyFromManifest(jarFile, path);
            if (dependency != null) {
                return dependency;
            }
        }
        return null;
    }

    private static Dependency loadDependencyFromJar(JarFile jarFile, String path, String subPath) throws Exception {
        InputStream in = jarFile.getInputStream(jarFile.getEntry(subPath));
        File outFile = new File(Config.getConfig().getBaseDirectory() + File.separator + "tmp");
        OutputStream out = new FileOutputStream(outFile);
        byte[] buffer = new byte[1024];
        int i;
        while ((i = in.read(buffer)) != -1) {
            out.write(buffer, 0, i);
        }
        out.flush();
        try {
            out.close();
            in.close();
        } catch (Throwable t) {
            // ignore
        }
        JarFile file = new JarFile(outFile);
        Dependency dependency = loadDependencyFromJarFile(file, path);
        file.close();
        return dependency;
    }

    private static Dependency loadDependencyFromPOM(JarFile jarFile, String path) throws Exception {
        InputStream in = readPomFromJarFile(jarFile);
        try {
            if (in != null) {
                Properties properties = new Properties();
                properties.load(in);
                String product = properties.getProperty("artifactId");
                String version = properties.getProperty("version");
                String vendor = properties.getProperty("groupId");
                if (product != null && version != null) {
                    return new Dependency(product, version, vendor, path, DEPENDENCY_SOURCE_POM);
                }
            }
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException io) {
                    LOGGER.warn("Error closing pom inputStream: ", io);
                }
            }
        }
        return null;
    }

    private static InputStream readPomFromJarFile(JarFile file) throws Exception {
        Enumeration<? extends ZipEntry> entries = file.entries();
        if (entries != null) {
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                if (entry.getName().startsWith("META-INF")
                        && entry.getName().endsWith("pom.properties")
                        && !entry.isDirectory()) {
                    return file.getInputStream(entry);
                }
            }
        }
        return null;
    }

    private static Dependency loadDependencyFromManifest(JarFile jarFile, String path) throws IOException {
        Manifest manifest = jarFile.getManifest();
        if (manifest != null) {
            Attributes attributes = manifest.getMainAttributes();
            String name = attributes.getValue("implementation-title");
            String version = attributes.getValue("implementation-version");
            String vendor = attributes.getValue("implementation-vendor-id");
            if (vendor == null) {
                vendor = attributes.getValue("implementation-vendor");
            }
            if (name != null && version != null) {
                return new Dependency(name, version, vendor, path, DEPENDENCY_SOURCE_MANEFEST_IMPL);
            } else {
                name = attributes.getValue("specification-title");
                version = attributes.getValue("specification-version");
                vendor = attributes.getValue("specification-vendor");
                if (name != null && version != null) {
                    return new Dependency(name, version, vendor, path, DEPENDENCY_SOURCE_MANEFEST_SPEC);
                } else {
                    name = attributes.getValue("bundle-symbolicname");
                    version = attributes.getValue("bundle-version");
                    vendor = attributes.getValue("bundle-vendor");
                    if (name != null && version != null) {
                        return new Dependency(name, version, vendor, path, DEPENDENCY_SOURCE_MANEFEST_BUNDLE);
                    }
                }
            }
        }
        return null;
    }

}
