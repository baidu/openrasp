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


import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.StackTrace;
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
            Scriptable params = null;
            try {
                Class reflectClass = (Class) Reflection.invokeMethod(method, "getDeclaringClass", new Class[]{});
                String reflectClassName = reflectClass.getName();
                String reflectMethodName = (String) Reflection.invokeMethod(method, "getName", new Class[]{});
                String absoluteMethodName = reflectClassName + "." + reflectMethodName;
                String[] reflectMonitorMethod = Config.getConfig().getReflectionMonitorMethod();
                for (String monitorMethod : reflectMonitorMethod) {
                    if (monitorMethod.equals(absoluteMethodName)) {
                        JSContext cx = JSContextFactory.enterAndInitContext();
                        params = cx.newObject(cx.getScope());
                        List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                                Config.getConfig().getPluginMaxStack());
                        Scriptable array = cx.newArray(cx.getScope(), stackInfo.toArray());
                        params.put("clazz", params, reflectClassName);
                        params.put("method", params, reflectMethodName);
                        params.put("stack", params, array);
                        break;
                    }
                }
            } finally {
                if (params != null) {
//                    HookHandler.doCheckWithoutRequest(CheckParameter.Type.REFLECTION, params);
                }
                HookHandler.enableCurrThreadHook();
            }
        }
    }
}