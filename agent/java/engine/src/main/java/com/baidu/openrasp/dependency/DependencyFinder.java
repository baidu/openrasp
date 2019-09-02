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

package com.baidu.openrasp.dependency;

import com.baidu.openrasp.ModuleLoader;
import com.baidu.openrasp.messaging.LogTool;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Properties;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import static com.baidu.openrasp.messaging.ErrorType.DEPENDENCY_ERROR;

/**
 * @description: 收集服务器部署项目的依赖信息
 * @author: anyang
 * @create: 2019/04/19 14:15
 */
public class DependencyFinder {
    public static final Logger LOGGER = Logger.getLogger(DependencyFinder.class.getPackage().getName() + ".log");

    public static HashSet<WeakReference<ClassLoader>> classLoaderCache = new HashSet<WeakReference<ClassLoader>>();
    private static final Pattern REGEX = Pattern.compile(".+/([^/]+)-(\\d[^/-]+)(?:-(?:\\w+))?\\.jar!/META-INF/MANIFEST\\.MF$");
    private static final Pattern JAR_LOCATION_REGEX = Pattern.compile("(?:file:)?(.+\\.jar)!/META-INF/MANIFEST\\.MF");

    static {
        classLoaderCache.add(new WeakReference<ClassLoader>(ModuleLoader.moduleClassLoader));
        classLoaderCache.add(new WeakReference<ClassLoader>(ClassLoader.getSystemClassLoader()));
    }

    public static HashSet<Dependency> getDependencySet() {
        HashSet<Dependency> dependencyHashSet = new HashSet<Dependency>();
        HashSet<WeakReference<ClassLoader>> classLoaders = new HashSet<WeakReference<ClassLoader>>(classLoaderCache);
        for (WeakReference<ClassLoader> classLoaderRef : classLoaders) {
            ClassLoader classLoader = classLoaderRef.get();
            Enumeration<URL> resources = null;
            try {
                if (classLoader != null) {
                    resources = classLoader.getResources("META-INF/MANIFEST.MF");
                } else {
                    classLoaderCache.remove(classLoaderRef);
                }
            } catch (Throwable e) {
                classLoaderCache.remove(classLoaderRef);
            }
            if (resources != null) {
                while (resources.hasMoreElements()) {
                    URL url = resources.nextElement();
                    String location = getJarLocation(url.getPath());
                    if (location != null) {
                        try {
                            Dependency dependency = getDependencyFromPOM(location);
                            if (dependency != null) {
                                dependencyHashSet.add(dependency);
                            } else {
                                dependency = getDependencyFromManifest(url, location);
                                if (dependency != null) {
                                    dependencyHashSet.add(dependency);
                                } else {
                                    dependency = getDependencyFromJar(url, location);
                                    if (dependency != null) {
                                        dependencyHashSet.add(dependency);
                                    } else {
                                        LOGGER.info("Could not find dependency information from jar path " + location);
                                    }
                                }
                            }
                        } catch (Exception e) {
                            LogTool.warn(DEPENDENCY_ERROR,
                                    "find dependency information failed from jar path " +
                                            location + ": " + e.getMessage());
                        }
                    }
                }
            }
        }
        return dependencyHashSet;
    }

    private static Dependency getDependencyFromPOM(String location) throws Exception {
        InputStream in = readZipFile(location);
        try {
            if (in != null) {
                Properties properties = new Properties();
                properties.load(in);
                String name = properties.getProperty("artifactId");
                String version = properties.getProperty("version");
                String vendor = properties.getProperty("groupId");
                if (name != null && version != null) {
                    return new Dependency(name, version, vendor, location);
                }
            }
            return null;
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException io) {
                    LOGGER.warn("Error closing pom inputStream: ", io);
                }
            }
        }
    }

    private static Dependency getDependencyFromManifest(URL url, String location) throws Exception {
        InputStream inputStream = null;
        try {
            inputStream = url.openStream();
            Manifest manifest = new Manifest(inputStream);
            Attributes attributes = manifest.getMainAttributes();
            String name = attributes.getValue("bundle-symbolicname");
            String version = attributes.getValue("bundle-version");
            String vendor = attributes.getValue("bundle-vendor");
            if (name != null && version != null) {
                return new Dependency(name, version, vendor, location);
            } else {
                name = attributes.getValue("implementation-title");
                version = attributes.getValue("implementation-version");
                vendor = attributes.getValue("implementation-vendor");
                if (name != null && version != null) {
                    return new Dependency(name, version, vendor, location);
                } else {
                    name = attributes.getValue("specification-title");
                    version = attributes.getValue("specification-version");
                    vendor = attributes.getValue("specification-vendor");
                    if (name != null && version != null) {
                        return new Dependency(name, version, vendor, location);
                    }
                }
            }
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException io) {
                    LOGGER.warn("Error closing manifest inputStream: ", io);
                }
            }
        }
        return null;
    }


    private static Dependency getDependencyFromJar(URL url, String location) {
        Matcher m = REGEX.matcher(url.getPath());
        if (m.matches()) {
            new Dependency(m.group(1), m.group(2), "", location);
        }
        return null;
    }

    private static InputStream readZipFile(String path) throws Exception {
        ZipFile file = new ZipFile(path);
        Enumeration<? extends ZipEntry> entries = file.entries();
        while (entries.hasMoreElements()) {
            ZipEntry entry = entries.nextElement();
            if (!entry.isDirectory() && entry.getName().startsWith("META-INF") && entry.getName().endsWith("pom.properties")) {
                return file.getInputStream(entry);
            }
        }
        return null;
    }

    private static String getJarLocation(String jarPath) {
        Matcher matcher = JAR_LOCATION_REGEX.matcher(jarPath);
        if (!matcher.matches()) {
            return null;
        }
        return matcher.group(1);
    }
}
