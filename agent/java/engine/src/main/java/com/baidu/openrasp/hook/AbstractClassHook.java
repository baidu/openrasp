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

package com.baidu.openrasp.hook;


import com.baidu.openrasp.config.Config;
import javassist.*;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.LinkedList;

/**
 * Created by zhuming01 on 5/19/17.
 * All rights reserved
 *
 * 用于向固定类的固定方法加钩子的类
 * 不同的hook点根据自己的需求实现该抽象类
 */
public abstract class AbstractClassHook {

    private static final Logger LOGGER = Logger.getLogger(AbstractClassHook.class.getName());

    protected boolean couldIgnore = true;

    private boolean isLoadedByBootstrapLoader = false;

    /**
     * 用于判断类名与当前需要hook的类是否相同
     *
     * @param className 用于匹配的类名
     * @return 是否匹配
     */
    public abstract boolean isClassMatched(String className);

    /**
     * hook点所属检测类型．
     *
     * @return 检测类型
     * @see <a href="https://rasp.baidu.com/doc/dev/data.html">https://rasp.baidu.com/doc/dev/data.html</a>
     */
    public abstract String getType();

    /**
     * hook 目标类的函数
     *
     * @param ctClass 目标类
     */
    protected abstract void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException;

    /**
     * 转化目标类
     *
     * @param ctClass 待转化的类
     * @return 转化之后类的字节码数组
     */
    public byte[] transformClass(CtClass ctClass) {
        try {
            hookMethod(ctClass);
            return ctClass.toBytecode();
        } catch (Exception e) {
            if (Config.getConfig().isDebugEnabled()) {
                LOGGER.error("transform class " + ctClass.getName() + " failed", e);
            }
        }
        return null;
    }

    /**
     * 是否可以在 hook.ignore 配置项中被忽略
     *
     * @return hook 点是否可别忽略
     */
    public boolean couldIgnore() {
        return couldIgnore;
    }

    /**
     * hook 点所在的类是否被 BootstrapClassLoader 加载
     *
     * @return true 代表是
     */
    public boolean isLoadedByBootstrapLoader() {
        return isLoadedByBootstrapLoader;
    }

    /**
     * 设置 hook 点所在的类是否被 BootstrapClassLoader 加载
     *
     * @param loadedByBootstrapLoader true 代表是
     */
    public void setLoadedByBootstrapLoader(boolean loadedByBootstrapLoader) {
        isLoadedByBootstrapLoader = loadedByBootstrapLoader;
    }

    /**
     * 在目标类的目标方法的入口插入相应的源代码
     *
     * @param ctClass    目标类
     * @param methodName 目标方法名称
     * @param desc       目标方法的描述符号
     * @param src        待插入的源代码
     */
    public void insertBefore(CtClass ctClass, String methodName, String desc, String src)
            throws NotFoundException, CannotCompileException {

        LinkedList<CtMethod> methods = getMethod(ctClass, methodName, desc);
        if (methods != null && methods.size() > 0) {
            for (CtMethod method : methods) {
                if (method != null) {
                    insertBefore(method, src);
                }
            }
        } else {
            if (Config.getConfig().isDebugEnabled()) {
                LOGGER.warn("can not find method " + methodName + " " + desc + " in class " + ctClass.getName());
            }
        }

    }

    /**
     * 在目标类的一组重载的目标方法的入口插入相应的源代码
     *
     * @param ctClass    目标类
     * @param methodName 目标方法名称
     * @param allDesc    目标方法的一组描述符
     * @param src        待插入的源代码
     */
    public void insertBefore(CtClass ctClass, String methodName, String src, String[] allDesc)
            throws NotFoundException, CannotCompileException {
        for (String desc : allDesc) {
            insertBefore(ctClass, methodName, desc, src);
        }
    }

    /**
     * 在目标类的目标方法的出口插入相应的源代码
     *
     * @param ctClass    目标类
     * @param methodName 目标方法名称
     * @param desc       目标方法的描述符号
     * @param src        待插入的源代码
     * @param asFinally  是否在抛出异常的时候同样执行该源代码
     */
    public void insertAfter(CtClass ctClass, String methodName, String desc, String src, boolean asFinally)
            throws NotFoundException, CannotCompileException {

        LinkedList<CtMethod> methods = getMethod(ctClass, methodName, desc);
        if (methods != null && methods.size() > 0) {
            for (CtMethod method : methods) {
                if (method != null) {
                    insertAfter(method, src, asFinally);
                }
            }
        } else {
            if (Config.getConfig().isDebugEnabled()) {
                LOGGER.warn("can not find method " + methodName + " " + desc + " in class " + ctClass.getName());
            }
        }

    }

    /**
     * 获取特定类的方法实例
     * 如果描述符为空，那么返回所有同名的方法
     *
     * @param ctClass    javassist 类实例
     * @param methodName 方法名称
     * @param desc       方法描述符
     * @return 所有符合要求的方法实例
     * @see javassist.bytecode.Descriptor
     */
    private LinkedList<CtMethod> getMethod(CtClass ctClass, String methodName, String desc) {
        LinkedList<CtMethod> methods = new LinkedList<CtMethod>();
        if (StringUtils.isEmpty(desc)) {
            CtMethod[] allMethods = ctClass.getDeclaredMethods();
            if (allMethods != null) {
                for (CtMethod method : allMethods) {
                    if (method.getName().equals(methodName)) {
                        methods.add(method);
                    }
                }
            }
        } else {
            try {
                methods.add(ctClass.getMethod(methodName, desc));
            } catch (NotFoundException e) {
                // ignore
            }
        }
        return methods;
    }

    /**
     * 在目标类的目标方法的入口插入相应的源代码
     *
     * @param method 目标方法
     * @param src    源代码
     */
    public void insertBefore(CtBehavior method, String src) throws CannotCompileException {
        try {
            method.insertBefore(src);
            LOGGER.info("insert before method " + method.getLongName());
        } catch (CannotCompileException e) {
            if (Config.getConfig().isDebugEnabled()) {
                LOGGER.error("insert before method " + method.getLongName() + " failed", e);
            }
            throw e;
        }
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#insertAfter(CtClass, String, String, String, boolean)
     */
    public void insertAfter(CtClass invokeClass, String methodName, String desc, String src)
            throws NotFoundException, CannotCompileException {
        insertAfter(invokeClass, methodName, desc, src, false);
    }

    /**
     * 在目标类的目标方法的出口插入相应的源代码
     *
     * @param method    目标方法
     * @param src       源代码
     * @param asFinally 是否在抛出异常的时候同样执行该源代码
     */
    public void insertAfter(CtBehavior method, String src, boolean asFinally) throws CannotCompileException {
        try {
            method.insertAfter(src, asFinally);
            LOGGER.info("insert after method: " + method.getLongName());
        } catch (CannotCompileException e) {
            LOGGER.error("insert after method " + method.getLongName() + " failed", e);
            throw e;
        }
    }

    /**
     * 获取调用静态方法的代码字符串
     *
     * @param invokeClass 静态方法所属的类
     * @param methodName  静态方法名称
     * @param paramString 调用传入的参数字符串,按照javassist格式
     * @return 整合之后的代码
     */
    public String getInvokeStaticSrc(Class invokeClass, String methodName, String paramString, Class... parameterTypes) {
        String src;
        String invokeClassName = invokeClass.getName();

        String parameterTypesString = "";
        if (parameterTypes != null && parameterTypes.length > 0) {
            for (Class parameterType : parameterTypes) {
                if (parameterType.getName().startsWith("[")) {
                    parameterTypesString += "Class.forName(\"" + parameterType.getName() + "\"),";
                } else {
                    parameterTypesString += (parameterType.getName() + ".class,");
                }
            }
            parameterTypesString = parameterTypesString.substring(0, parameterTypesString.length() - 1);
        }
        if (parameterTypesString.equals("")) {
            parameterTypesString = null;
        } else {
            parameterTypesString = "new Class[]{" + parameterTypesString + "}";
        }
        src = "com.baidu.openrasp.ModuleLoader.loadClass(\"" + invokeClassName + "\").getMethod(\"" + methodName +
                "\"," + parameterTypesString + ").invoke(null";
        if (!StringUtils.isEmpty(paramString)) {
            src += (",new Object[]{" + paramString + "});");
        } else {
            src += ",null);";
        }
        src = "try {" + src + "} catch (Throwable t) {if(t.getClass().getName().equals" +
                "(\"com.baidu.openrasp.exception.SecurityException\")){throw t;}}";
        return src;
    }

}
