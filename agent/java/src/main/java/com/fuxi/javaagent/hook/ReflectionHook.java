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

package com.fuxi.javaagent.hook;


import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import com.fuxi.javaagent.tool.Reflection;
import com.fuxi.javaagent.tool.StackTrace;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.List;


/**
 * Created by tyy on 9/13/17.
 * 反射hook类
 */
public class ReflectionHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return className.equals("java/lang/reflect/Method");
    }

    @Override
    public String getType() {
        return "reflection";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if ("invoke".equals(name)) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadThis();
                    invokeStatic(Type.getType(ReflectionHook.class),
                            new Method("checkReflection",
                                    "(Ljava/lang/Object;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 反射hook点检测
     *
     * @param method 反射调用的方法
     */
    public static void checkReflection(Object method) {
        if (HookHandler.enableHook.get() && HookHandler.isEnableCurrThreadHook()) {
            HookHandler.disableCurrThreadHook();
            try {
                Class reflectClass = (Class) Reflection.invokeMethod(method, "getDeclaringClass", new Class[]{});
                String reflectClassName = reflectClass.getName();
                String reflectMethodName = (String) Reflection.invokeMethod(method, "getName", new Class[]{});
                String absoluteMethodName = reflectClassName + "." + reflectMethodName;
                String[] reflectMonitorMethod = Config.getConfig().getReflectionMonitorMethod();
                for (String monitorMethod : reflectMonitorMethod) {
                    if (monitorMethod.equals(absoluteMethodName)) {
                        JSContext cx = JSContextFactory.enterAndInitContext();
                        Scriptable params = cx.newObject(cx.getScope());
                        List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                                Config.getConfig().getReflectionMaxStack());
                        Scriptable array = cx.newArray(cx.getScope(), stackInfo.toArray());
                        params.put("clazz", params, reflectClassName);
                        params.put("method", params, reflectMethodName);
                        params.put("stack", params, array);
                        HookHandler.doCheckWithoutRequest(CheckParameter.Type.REFLECTION, params);
                        break;
                    }
                }
            } finally {
                HookHandler.enableCurrThreadHook();
            }
        }
    }
}