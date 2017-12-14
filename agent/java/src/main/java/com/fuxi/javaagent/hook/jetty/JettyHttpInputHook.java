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

package com.fuxi.javaagent.hook.jetty;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.hook.AbstractClassHook;
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
