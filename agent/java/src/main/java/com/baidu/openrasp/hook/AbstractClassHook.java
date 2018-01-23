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


import org.objectweb.asm.*;
import org.objectweb.asm.commons.JSRInlinerAdapter;
import org.apache.log4j.Logger;

/**
 * Created by zhuming01 on 5/19/17.
 * All rights reserved
 * <p>
 * 用于向固定类的固定方法加钩子的类
 * 不同的hook点根据自己的需求实现该抽象类
 */
public abstract class AbstractClassHook {
    private static final Logger LOGGER = Logger.getLogger(AbstractClassHook.class.getName());

    private String[] interfaces;

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
     * 是否计算栈帧
     *
     * @return true计算，false不计算
     */
    protected boolean computeFrames() {
        return false;
    }

    /**
     * 转化待转化的类
     *
     * @param className       待转化的类名称
     * @param classfileBuffer 带转化的类的字节码数组
     * @return 转化后的类的字节码数组
     */
    public byte[] transformClass(String className, byte[] classfileBuffer) {
        try {
            ClassReader reader = new ClassReader(classfileBuffer);
            ClassWriter writer = new ClassWriter(reader, computeFrames() ? ClassWriter.COMPUTE_FRAMES : ClassWriter.COMPUTE_MAXS);
            LOGGER.debug("transform class: " + className);
            ClassVisitor visitor = new RaspHookClassVisitor(this, writer);
            reader.accept(visitor, ClassReader.EXPAND_FRAMES);
            return writer.toByteArray();
        } catch (RuntimeException e) {
            LOGGER.error("exception", e);
        }
        return null;
    }

    public String[] getInterfaces() {
        return interfaces;
    }

    public void setInterfaces(String[] interfaces) {
        this.interfaces = interfaces;
    }

    /**
     * 向hook点函数增加的钩子内容
     *
     * @param access     方法访问标志
     * @param name       方法名称
     * @param desc       方法描述符
     * @param signature  方法的签名
     * @param exceptions 方法抛出的异常
     * @param mv         传递到本方法的代理method visitor
     * @return 转化后的MethodVisitor用于传递到下一个解析该方法的MethodVisitor
     */
    protected abstract MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv);

    /**
     * 继承自ClassVisitor用于回调读取class字节码时产生的事件
     */
    private static class RaspHookClassVisitor extends ClassVisitor {
        private AbstractClassHook hook;

        /**
         * constructor
         *
         * @param hook 用于添加类中方法的hook点
         * @param cv   {@link ClassVisitor}
         */
        RaspHookClassVisitor(AbstractClassHook hook, ClassVisitor cv) {
            super(Opcodes.ASM5, cv);
            this.hook = hook;
        }

        @Override
        public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
            hook.setInterfaces(interfaces);
            super.visit(version, access, name, signature, superName, interfaces);
        }

        /**
         * (none-javadoc)
         *
         * @see ClassVisitor#visitMethod(int, String, String, String, String[])
         */
        @Override
        public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
            MethodVisitor mv = super.visitMethod(access, name, desc, signature, exceptions);
            if (hook.computeFrames()) {
                mv = new JSRInlinerAdapter(mv, access, name, desc, signature, exceptions);
            }

            MethodVisitor hmv = hook.hookMethod(access, name, desc, signature, exceptions, mv);
            if (hmv != mv && LOGGER.isDebugEnabled()) {
                LOGGER.debug("hook method: " + name + ":" + desc);
            }
            return hmv;
        }
    }
}
