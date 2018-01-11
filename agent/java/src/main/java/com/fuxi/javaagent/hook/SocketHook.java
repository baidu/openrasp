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

    private static String CLASS_NAME = "java/net/Socket";

    static {
        AbstractClassHook.HOOKED_CLASSES.add(CLASS_NAME.replaceAll("/", "."));
    }

    @Override
    public boolean isClassMatched(String className) {
        return className.equals(CLASS_NAME);
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
