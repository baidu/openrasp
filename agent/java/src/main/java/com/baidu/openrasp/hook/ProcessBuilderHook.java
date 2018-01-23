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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.StackTrace;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.List;

/**
 * Created by zhuming01 on 5/17/17.
 * All rights reserved
 */
public class ProcessBuilderHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "command";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/lang/ProcessBuilder".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("start") && desc.equals("()Ljava/lang/Process;")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadThis();
                    invokeVirtual(Type.getType("java/lang/ProcessBuilder"),
                            new Method("command", "()Ljava/util/List;"));
                    invokeStatic(Type.getType(ProcessBuilderHook.class),
                            new Method("checkCommand", "(Ljava/util/List;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 命令执行hook点
     *
     * @param command 命令列表
     */
    public static void checkCommand(List<String> command) {
        if (command != null && !command.isEmpty()) {
            Scriptable params = null;
            try {
                JSContext cx = JSContextFactory.enterAndInitContext();
                params = cx.newObject(cx.getScope());
                Scriptable commandArray = cx.newArray(cx.getScope(), command.toArray());
                params.put("command", params, commandArray);
                List<String> stackInfo = StackTrace.getStackTraceArray(Config.REFLECTION_STACK_START_INDEX,
                        Config.getConfig().getPluginMaxStack());
                Scriptable stackArray = cx.newArray(cx.getScope(), stackInfo.toArray());
                params.put("stack", params, stackArray);
            } catch (Throwable t) {
                HookHandler.LOGGER.warn(t.getMessage());
            }
            if(params != null) {
                HookHandler.doCheck(CheckParameter.Type.COMMAND, params);
            }
        }
    }
}
