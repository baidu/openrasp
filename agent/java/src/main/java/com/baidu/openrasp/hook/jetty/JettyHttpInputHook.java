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

package com.baidu.openrasp.hook.jetty;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 9/25/17.
 *
 * 获取 jetty 请求 body 的 hook 点
 */
public class JettyHttpInputHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("org/eclipse/jetty/server/HttpInput");
    }

    @Override
    public String getType() {
        return "body";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, final String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("read")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodExit(int opcode) {
                    if (opcode == Opcodes.IRETURN) {
                        if (desc.equals("()I")) {
                            dup();
                            loadThis();
                            invokeStatic(Type.getType(HookHandler.class),
                                    new Method("onInputStreamRead", "(ILjava/lang/Object;)V"));
                        } else if (desc.equals("([BII)I")) {
                            dup();
                            loadThis();
                            loadArg(0);
                            loadArg(1);
                            loadArg(2);
                            invokeStatic(Type.getType(HookHandler.class),
                                    new Method("onInputStreamRead", "(ILjava/lang/Object;[BII)V"));
                        }
                    }
                    super.onMethodExit(opcode);
                }
            };
        }
        return mv;
    }
}
