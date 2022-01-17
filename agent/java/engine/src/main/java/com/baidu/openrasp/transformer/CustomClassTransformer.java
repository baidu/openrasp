/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.transformer;

import com.baidu.openrasp.ModuleLoader;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.dependency.DependencyFinder;
import com.baidu.openrasp.detector.ServerDetectorManager;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.annotation.AnnotationScanner;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.ClassClassPath;
import javassist.ClassPool;
import javassist.CtClass;
import javassist.LoaderClassPath;
import org.apache.log4j.Logger;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.lang.instrument.Instrumentation;
import java.lang.ref.SoftReference;
import java.security.ProtectionDomain;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListSet;

/**
 * 自定义类字节码转换器，用于hook类的方法
 */
public class CustomClassTransformer implements ClassFileTransformer {
    public static final Logger LOGGER = Logger.getLogger(CustomClassTransformer.class.getName());
    private static final String SCAN_ANNOTATION_PACKAGE = "com.baidu.openrasp.hook";
    private static HashSet<String> jspClassLoaderNames = new HashSet<String>();
    private static ConcurrentSkipListSet<String> necessaryHookType = new ConcurrentSkipListSet<String>();
    private static ConcurrentSkipListSet<String> dubboNecessaryHookType = new ConcurrentSkipListSet<String>();
    public static ConcurrentHashMap<String, SoftReference<ClassLoader>> jspClassLoaderCache = new ConcurrentHashMap<String, SoftReference<ClassLoader>>();

    private Instrumentation inst;
    private HashSet<AbstractClassHook> hooks = new HashSet<AbstractClassHook>();
    private ServerDetectorManager serverDetector = ServerDetectorManager.getInstance();

    public static volatile boolean isNecessaryHookComplete = false;
    public static volatile boolean isDubboNecessaryHookComplete = false;

    static {
        jspClassLoaderNames.add("org.apache.jasper.servlet.JasperLoader");
        jspClassLoaderNames.add("com.caucho.loader.DynamicClassLoader");
        jspClassLoaderNames.add("com.ibm.ws.jsp.webcontainerext.JSPExtensionClassLoader");
        jspClassLoaderNames.add("weblogic.servlet.jsp.JspClassLoader");
        jspClassLoaderNames.add("com.tongweb.jasper.servlet.JasperLoader");
        dubboNecessaryHookType.add("dubbo_preRequest");
        dubboNecessaryHookType.add("dubboRequest");
    }

    public CustomClassTransformer(Instrumentation inst) {
        this.inst = inst;
        inst.addTransformer(this, true);
        addAnnotationHook();
    }

    public void release() {
        inst.removeTransformer(this);
        retransform();
    }

    public void retransform() {
        LinkedList<Class> retransformClasses = new LinkedList<Class>();
        Class[] loadedClasses = inst.getAllLoadedClasses();
        for (Class clazz : loadedClasses) {
            if (isClassMatched(clazz.getName().replace(".", "/"))) {
                if (inst.isModifiableClass(clazz) && !clazz.getName().startsWith("java.lang.invoke.LambdaForm")) {
                    try {
                        // hook已经加载的类，或者是回滚已经加载的类
                        inst.retransformClasses(clazz);
                    } catch (Throwable t) {
                        LogTool.error(ErrorType.HOOK_ERROR,
                                "failed to retransform class " + clazz.getName() + ": " + t.getMessage(), t);
                    }
                }
            }
        }
    }

    private void addHook(AbstractClassHook hook, String className) {
        if (hook.isNecessary()) {
            necessaryHookType.add(hook.getType());
        }
        String[] ignore = Config.getConfig().getIgnoreHooks();
        for (String s : ignore) {
            if (hook.couldIgnore() && (s.equals("all") || s.equals(hook.getType()))) {
                LOGGER.info("ignore hook type " + hook.getType() + ", class " + className);
                return;
            }
        }
        hooks.add(hook);
    }

    private void addAnnotationHook() {
        Set<Class> classesSet = AnnotationScanner.getClassWithAnnotation(SCAN_ANNOTATION_PACKAGE, HookAnnotation.class);
        for (Class clazz : classesSet) {
            try {
                Object object = clazz.newInstance();
                if (object instanceof AbstractClassHook) {
                    addHook((AbstractClassHook) object, clazz.getName());
                }
            } catch (Exception e) {
                LogTool.error(ErrorType.HOOK_ERROR, "add hook failed: " + e.getMessage(), e);
            }
        }
    }

    /**
     * 过滤需要hook的类，进行字节码更改
     *
     * @see ClassFileTransformer#transform(ClassLoader, String, Class, ProtectionDomain, byte[])
     */
    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined,
                            ProtectionDomain domain, byte[] classfileBuffer) throws IllegalClassFormatException {
        if (loader != null) {
            DependencyFinder.addJarPath(domain);
        }
        if (loader != null && jspClassLoaderNames.contains(loader.getClass().getName())) {
            jspClassLoaderCache.put(className.replace("/", "."), new SoftReference<ClassLoader>(loader));
        }
        for (final AbstractClassHook hook : hooks) {
            if (hook.isClassMatched(className)) {
                CtClass ctClass = null;
                try {
                    ClassPool classPool = new ClassPool();
                    addLoader(classPool, loader);
                    ctClass = classPool.makeClass(new ByteArrayInputStream(classfileBuffer));
                    if (loader == null) {
                        hook.setLoadedByBootstrapLoader(true);
                    }
                    classfileBuffer = hook.transformClass(ctClass);
                    if (classfileBuffer != null) {
                        checkNecessaryHookType(hook.getType());
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    if (ctClass != null) {
                        ctClass.detach();
                    }
                }
            }
        }
        serverDetector.detectServer(className, loader, domain);
        return classfileBuffer;
    }

    private void checkNecessaryHookType(String type) {
        if (!isNecessaryHookComplete && necessaryHookType.contains(type)) {
            necessaryHookType.remove(type);
            if (necessaryHookType.isEmpty()) {
                isNecessaryHookComplete = true;
            }
        }

        if (!isDubboNecessaryHookComplete && dubboNecessaryHookType.contains(type)) {
            dubboNecessaryHookType.remove(type);
            if (dubboNecessaryHookType.isEmpty()) {
                isDubboNecessaryHookComplete = true;
            }
        }
    }


    public boolean isClassMatched(String className) {
        for (final AbstractClassHook hook : getHooks()) {
            if (hook.isClassMatched(className)) {
                return true;
            }
        }
        return serverDetector.isClassMatched(className);
    }

    private void addLoader(ClassPool classPool, ClassLoader loader) {
        classPool.appendSystemPath();
        classPool.appendClassPath(new ClassClassPath(ModuleLoader.class));
        if (loader != null) {
            classPool.appendClassPath(new LoaderClassPath(loader));
        }
    }

    public HashSet<AbstractClassHook> getHooks() {
        return hooks;
    }

}
