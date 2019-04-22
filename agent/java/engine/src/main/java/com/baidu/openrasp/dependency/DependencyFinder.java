package com.baidu.openrasp.dependency;

import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @description: 收集服务器部署项目的依赖信息
 * @author: anyang
 * @create: 2019/04/19 14:15
 */
public class DependencyFinder {
    public static final Logger LOGGER = Logger.getLogger(DependencyFinder.class.getPackage().getName() + ".log");

    public static HashSet<ClassLoader> classLoaderCache = new HashSet<ClassLoader>();
    private static final Pattern REGEX = Pattern.compile(".+/([^/]+)-(\\d[^/-]+)(?:-(?:\\w+))?\\.jar!/META-INF/MANIFEST\\.MF$");
    private static final Pattern JAR_LOCATION_REGEX = Pattern.compile("(?:file:)?(.+\\.jar)!/META-INF/MANIFEST\\.MF");

    static {
        classLoaderCache.add(ClassLoader.getSystemClassLoader());
    }

    public static HashSet<Dependency> getDependencySet() {
        HashSet<Dependency> dependencyHashSet = new HashSet<Dependency>();
        HashSet<ClassLoader> classLoaders = new HashSet<ClassLoader>(classLoaderCache);
        for (ClassLoader classLoader : classLoaders) {
            Enumeration<URL> resources = null;
            try {
                resources = classLoader.getResources("META-INF/MANIFEST.MF");
            } catch (Throwable e) {
                if (e instanceof IllegalStateException) {
                    classLoaderCache.remove(classLoader);
                } else {
                    String message = "Could not find manifest";
                    int errorCode = ErrorType.DEPENDENCY_ERROR.getCode();
                    LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                }
            }
            if (resources != null) {
                while (resources.hasMoreElements()) {
                    URL url = resources.nextElement();
                    String location = getJarLocation(url.getPath());
                    InputStream inputStream = null;
                    try {
                        inputStream = url.openStream();
                        Manifest manifest = new Manifest(inputStream);
                        Attributes attributes = manifest.getMainAttributes();
                        String name = attributes.getValue("bundle-symbolicname");
                        String version = attributes.getValue("bundle-version");
                        if (name != null && version != null) {
                            dependencyHashSet.add(new Dependency(name, version, location));
                        } else {
                            name = attributes.getValue("implementation-title");
                            version = attributes.getValue("implementation-version");
                            if (name != null && version != null) {
                                dependencyHashSet.add(new Dependency(name, version, location));
                            } else {
                                Matcher m = REGEX.matcher(url.getPath());
                                if (m.matches()) {
                                    dependencyHashSet.add(new Dependency(m.group(1), m.group(2), location));
                                } else {
                                    LOGGER.info("Could not find dependency information from jar path " + url.getPath());
                                }
                            }
                        }
                        if (inputStream != null) {
                            try {
                                inputStream.close();
                            } catch (IOException e) {
                                LOGGER.warn("Error closing manifest inputStream: ", e);
                            }
                        }
                    } catch (IOException e) {
                        String message = "find dependency information failed from jar path " + location;
                        int errorCode = ErrorType.DEPENDENCY_ERROR.getCode();
                        LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                    } finally {
                        if (inputStream != null) {
                            try {
                                inputStream.close();
                            } catch (IOException io) {
                                LOGGER.warn("Error closing manifest inputStream: ", io);
                            }
                        }
                    }
                }
            }
        }
        return dependencyHashSet;
    }

    private static String getJarLocation(String jarPath) {
        Matcher matcher = JAR_LOCATION_REGEX.matcher(jarPath);
        if (!matcher.matches()) {
            return null;
        }
        return matcher.group(1);
    }
}
