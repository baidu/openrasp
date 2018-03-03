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
package com.fuxi.javaagent.hook;


import com.fuxi.javaagent.plugin.checker.local.TrustStringManager;
import com.fuxi.javaagent.config.Config;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by fastdev@163.com on 2/4/18.
 * All rights reserved
 */
public class ConstStringHook extends AbstractClassHook {
    private static Type ReplaceBuilderType = Type.getType(TrustStringManager.class);
    @Override
    public boolean isClassMatched(String className) {
        String scanPackage = Config.getConfig().getSqlInjectScanClassPrefix();
        if(scanPackage == null || "".equals(scanPackage)) {
            return false;
        }
        return className.startsWith(scanPackage);
    }

    @Override
    public String getType() {
        return "ConstStringHook";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
            @Override
            public void visitLdcInsn(final Object cst) {
                super.visitLdcInsn(cst);
                if(cst != null && cst.getClass().equals(String.class)) {
                    invokeStatic(Type.getType(TrustStringManager.class), new Method("getConstString", "(Ljava/lang/String;)Ljava/lang/String;"));
                }
            }

            public void visitMethodInsn(int opcode, String owner, String name,
                                        String desc, boolean itf) {
                if(opcode == Opcodes.INVOKEVIRTUAL) {
                    if(owner.equals("java/lang/StringBuilder")) {
                        if(("append").equals(name) && (("(Ljava/lang/String;)Ljava/lang/StringBuilder;").equals(desc)
                                || ("(Ljava/lang/StringBuffer;)Ljava/lang/StringBuilder;").equals(desc)
                                || ("(Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;").equals(desc)
                        )) {
                            invokeStatic(ReplaceBuilderType, new Method("handleBuilderAdd", "(Ljava/lang/StringBuilder;Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;"));
                            return;
                        }
                        else if(("append").equals(name) && ("(C)Ljava/lang/StringBuilder;").equals(desc)) {
                            invokeStatic(ReplaceBuilderType, new Method("handleBuilderAdd", "(Ljava/lang/StringBuilder;C)Ljava/lang/StringBuilder;"));
                            return;
                        }
                        else if (("toString").equals(name) && ("()Ljava/lang/String;").equals(desc)) {
                            invokeStatic(ReplaceBuilderType, new Method("handleBuilderToString", "(Ljava/lang/StringBuilder;)Ljava/lang/String;"));
                            return;
                        }
                    }
                }
                super.visitMethodInsn(opcode, owner, name,
                        desc, itf);
            }
        };
    }
}