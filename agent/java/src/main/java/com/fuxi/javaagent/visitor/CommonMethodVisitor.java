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

import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;

/**
 * Created by tyy on 4/6/17.
 * 基于事件的类的字节码解析器
 */
public class CommonMethodVisitor extends AdviceAdapter {
    private MethodVisitor mv;
    private String className;
    private String methodName;
    private String methodDesc;
    private int index;
    private Type[] argumentTypes;

    /**
     * (none-javadoc)
     *
     * @see #CommonMethodVisitor(int, MethodVisitor, String, int, String, String)
     */
    public CommonMethodVisitor(MethodVisitor mv, String className, int access, String name, String desc) {
        this(Opcodes.ASM5, mv, className, access, name, desc);
    }

    /**
     * 根据方法的描述符初始化参数Type数组
     *
     * @see AdviceAdapter#AdviceAdapter(int, MethodVisitor, int, String, String)
     */
    private CommonMethodVisitor(int api, MethodVisitor mv, String className, int access, String name, String desc) {
        super(api, mv, access, name, desc);
        this.mv = mv;
        this.className = className;
        this.methodName = name;
        this.methodDesc = desc;

        this.index = ((access & Opcodes.ACC_STATIC) != 0 ? 0 : 1);
        this.argumentTypes = Type.getArgumentTypes(desc);
    }

    /**
     * 把hook点参数传递到检测插件中
     *
     * @see AdviceAdapter#AdviceAdapter(int, MethodVisitor, int, String, String)
     */
    @Override
    public void onMethodEnter() {
        int argumentSize = argumentTypes.length;
        if (argumentSize + index > 5) {
            System.err.println("argument size > 5: " + className + "." + methodName + ":" + methodDesc);
            argumentSize = 5 - index;
        }

        mv.visitLdcInsn(this.className);
        mv.visitLdcInsn(this.methodName);
        mv.visitLdcInsn(this.methodDesc);

        mv.visitInsn(Opcodes.ICONST_0 + argumentSize + index); // array length --> stack
        mv.visitTypeInsn(Opcodes.ANEWARRAY, "java/lang/Object"); // new array --> stack

        if (index == 1) {
            mv.visitInsn(DUP);
            mv.visitInsn(ICONST_0); // i(array index) --> stack
            mv.visitVarInsn(Opcodes.ALOAD, 0); // 'this' pointer --> stack
            mv.visitInsn(AASTORE); // array[0] = 'this' pointer
        }
        int base = ICONST_0 + index;
        for (int i = 0; i < argumentSize; i++) {
            mv.visitInsn(DUP);
            mv.visitInsn(base + i); // i(array index) --> stack

            switch (argumentTypes[i].getSort()) {
                case Type.BOOLEAN:
                    mv.visitVarInsn(ILOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Boolean", "valueOf", "(Z)Ljava/lang/Boolean;", false);
                    index += 1;
                    break;
                case Type.CHAR:
                    mv.visitVarInsn(ILOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Character", "valueOf", "(C)Ljava/lang/Character;", false);
                    index += 1;
                    break;
                case Type.BYTE:
                    mv.visitVarInsn(ILOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Byte", "valueOf", "(B)Ljava/lang/Byte;", false);
                    index += 1;
                    break;
                case Type.SHORT:
                    mv.visitVarInsn(ILOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Short", "valueOf", "(S)Ljava/lang/Short;", false);
                    index += 1;
                    break;
                case Type.INT:
                    mv.visitVarInsn(ILOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Integer", "valueOf", "(I)Ljava/lang/Integer;", false);
                    index += 1;
                    break;
                case Type.FLOAT:
                    mv.visitVarInsn(FLOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Float", "valueOf", "(F)Ljava/lang/Float;", false);
                    index += 1;
                    break;
                case Type.DOUBLE:
                    mv.visitVarInsn(DLOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Double", "valueOf", "(D)Ljava/lang/Double;", false);
                    index += 2;
                    break;
                case Type.LONG:
                    mv.visitVarInsn(LLOAD, index);
                    mv.visitMethodInsn(INVOKESTATIC, "java/lang/Long", "valueOf", "(J)Ljava/lang/Long;", false);
                    index += 2;
                    break;
                default:
                    mv.visitVarInsn(Opcodes.ALOAD, index); // parameter #i --> stack
                    index += 1;
                    break;
            }
            mv.visitInsn(AASTORE); // array[i] = #i
        }
        mv.visitMethodInsn(Opcodes.INVOKESTATIC,
                "com/fuxi/javaagent/HookHandler", "checkCommon",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)V", false);
    }
}
