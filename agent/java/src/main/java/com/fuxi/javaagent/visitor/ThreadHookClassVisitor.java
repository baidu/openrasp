/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.visitor;

import org.apache.log4j.Logger;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Callable;


/**
 * Created by tyy on 4/26/17.
 * All rights reserved
 * hook所有Runnable和Callable的子类，用于记录请求子线程中的请求缓存
 */
public class ThreadHookClassVisitor extends ClassVisitor {
    private static final Logger LOGGER = Logger.getLogger(ThreadHookClassVisitor.class.getName());
    private static Set<String> callableClasses;
    private static Set<String> runnableClasses;

    private String className;
    private ClassVisitor cv;


    private boolean isRunnable = false;
    private boolean isCallable = false;

    static {
        callableClasses = new HashSet<String>();
        runnableClasses = new HashSet<String>();

        callableClasses.add("java/util/concurrent/Callable");
        runnableClasses.add("java/lang/Runnable");
    }

    public ThreadHookClassVisitor(ClassVisitor cv) {
        super(Opcodes.ASM5, cv);
        this.cv = cv;
    }

    /**
     * 判断类是否为Runnable或者Callable的子类
     *
     * @see ClassVisitor#visit(int, int, String, String, String, String[])
     */
    @Override
    public void visit(int version, int access, String name, String signature,
                      String superName, String[] interfaces) {
        this.className = name;
        this.cv.visit(version, access, name, signature, superName, interfaces);

        this.isRunnable = checkIsRunnable(superName, interfaces);
        this.isCallable = checkIsCallable(superName, interfaces);
        if (this.isRunnable) {
            addRunnable(name);
        }
        if (this.isCallable) {
            addCallable(name);
        }
    }

    /**
     * 检查是否是Runnable的子类
     *
     * @param superName  继承父类名称
     * @param interfaces 实现的接口列表
     * @return true是Runnable子类，false不是
     */
    private boolean checkIsRunnable(String superName, String[] interfaces) {
        try {
            // 首先尝试加载父类，如果成功判断是否实现了Runnable接口
            Class clazz = Class.forName(superName.replace('/', '.'));
            return Runnable.class.isAssignableFrom(clazz);
        } catch (ClassNotFoundException ignored) {
        }
        // 如果父类加载失败，根据父类名和接口名判断
        return checkWithinRunnable(superName, interfaces);
    }

    /**
     * 检查是否是Callable的子类
     *
     * @param superName  继承父类名称
     * @param interfaces 实现的接口列表
     * @return true是Callable子类，false不是
     */
    private boolean checkIsCallable(String superName, String[] interfaces) {
        try {
            Class clazz = Class.forName(superName.replace('/', '.'));
            return Callable.class.isAssignableFrom(clazz);
        } catch (ClassNotFoundException ignored) {
        }
        return checkWithinCallable(superName, interfaces);
    }

    // 检查super class或者interface是否在Callable集合中
    private static synchronized boolean checkWithinCallable(String superName, String[] interfaces) {
        return callableClasses.contains(superName) || containsAny(callableClasses, interfaces);
    }

    // 检查super class或者interface是否Runnable在
    private static synchronized boolean checkWithinRunnable(String superName, String[] interfaces) {
        return runnableClasses.contains(superName) || containsAny(callableClasses, interfaces);
    }

    private static synchronized void addCallable(String className) {
        LOGGER.debug("add callable class: " + className);
        callableClasses.add(className);
    }

    private static synchronized void addRunnable(String className) {
        LOGGER.debug("add runnable class: " + className);
        runnableClasses.add(className);
    }

    private static boolean containsAny(Set<String> set, String[] arr) {
        for (String a : arr) {
            if (set.contains(a)) {
                return true;
            }
        }
        return false;
    }

    /**
     * hook线程子类
     *
     * @see ClassVisitor#visitMethod(int, String, String, String, String[])
     */
    @Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
        MethodVisitor mv = super.visitMethod(access, name, desc, signature, exceptions);
        if (isRunnable && name.equals("run") && desc.equals("()V")) {
            return new CommonMethodVisitor(mv, this.className, access, name, desc);
        }
        if (isCallable && name.equals("call") && desc.equals("()V")) {
            return new CommonMethodVisitor(mv, this.className, access, name, desc);
        }
        return mv;
    }
}
