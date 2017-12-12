/*
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

package com.fuxi.javaagent.hook;


import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.net.InetSocketAddress;
import java.net.SocketAddress;

/**
 * Created by tyy on 17-11-17.
 * 检测socket连接的hook点
 */
public class SocketHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return className.equals("java/net/Socket");
    }

    @Override
    public String getType() {
        return "socket";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (("connect").equals(name) && ("(Ljava/net/SocketAddress;I)V").equals(desc)) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(0);
                    invokeStatic(Type.getType(SocketHook.class),
                            new Method("checkSocketHost", "(Ljava/net/SocketAddress;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 检测socket连接的host
     *
     * @param address socket连接地址
     */
    public static void checkSocketHost(SocketAddress address) {
        try {
            if (address != null && address instanceof InetSocketAddress) {
                String hostName = ((InetSocketAddress) address).getHostName();
                JSContext cx = JSContextFactory.enterAndInitContext();
                Scriptable params = cx.newObject(cx.getScope());
                params.put("hostname", params, hostName);
                HookHandler.doCheck(CheckParameter.Type.SSRF, params);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
