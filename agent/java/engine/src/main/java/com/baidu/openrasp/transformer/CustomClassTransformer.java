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

package com.baidu.openrasp.transformer;

import com.baidu.openrasp.ModuleLoader;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.detector.ServerDetectorManager;
import com.baidu.openrasp.hook.AbstractClassHook;
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
import java.lang.instrument.UnmodifiableClassException;
import java.lang.ref.WeakReference;
import java.security.ProtectionDomain;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 自定义类字节码转换器，用于hook类德 方法
 */
public class CustomClassTransformer implements ClassFileTransformer {
    public static final Logger LOGGER = Logger.getLogger(CustomClassTransformer.class.getName());
    private static final String SCAN_ANNOTATION_PACKAGE = "com.baidu.openrasp.hook";
    private static HashSet<String> jspClassLoaderNames = new HashSet<String>();
    public static ConcurrentHashMap<String, WeakReference<ClassLoader>> jspClassLoaderCache = new ConcurrentHashMap<String, WeakReference<ClassLoader>>();

    private Instrumentation inst;
    private HashSet<AbstractClassHook> hooks = new HashSet<AbstractClassHook>();
    private ServerDetectorManager serverDetector = ServerDetectorManager.getInstance();

    static {
        jspClassLoaderNames.add("org.apache.jasper.servlet.JasperLoader");
        jspClassLoaderNames.add("com.caucho.loader.DynamicClassLoader");
        jspClassLoaderNames.add("com.ibm.ws.jsp.webcontainerext.JSPExtensionClassLoader");
        jspClassLoaderNames.add("weblogic.servlet.jsp.JspClassLoader");
    }

    public CustomClassTransformer(Instrumentation inst) {
        this.inst = inst;
        inst.addTransformer(this, true);
        addAnnotationHook();
    }

    public void release() {
        inst.removeTransformer(this);
        try {
            retransform();
        } catch (UnmodifiableClassException e) {
            int errorCode = ErrorType.HOOK_ERROR.getCode();
            LOGGER.error(CloudUtils.getExceptionObject("retransform classes failed", errorCode), e);
        }
    }

    public void retransform() throws UnmodifiableClassException {
        LinkedList<Class> retransformClasses = new LinkedList<Class>();
        Class[] loadedClasses = inst.getAllLoadedClasses();
        for (Class clazz : loadedClasses) {
            if (isClassMatched(clazz.getName().replace(".", "/"))) {
                if (inst.isModifiableClass(clazz) && !clazz.getName().startsWith("java.lang.invoke.LambdaForm")) {
                    retransformClasses.add(clazz);
                }
            }
        }
        // hook已经加载的类，或者是回滚已经加载的类
        Class[] classes = new Class[retransformClasses.size()];
        retransformClasses.toArray(classes);
        if (classes.length > 0) {
            inst.retransformClasses(classes);
        }
    }

    private void addHook(AbstractClassHook hook, String className) {
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
                String message = "add hook failed";
                int errorCode = ErrorType.HOOK_ERROR.getCode();
                LOGGER.error(CloudUtils.getExceptionObject(message, errorCode), e);
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
        if (loader != null && jspClassLoaderNames.contains(loader.getClass().getName())) {
            jspClassLoaderCache.put(className.replace("/", "."), new WeakReference<ClassLoader>(loader));
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
