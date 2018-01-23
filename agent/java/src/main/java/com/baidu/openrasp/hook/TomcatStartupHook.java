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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 8/9/17.
 * 用于hook tomcat启动函数
 */
public class TomcatStartupHook extends AbstractClassHook {

    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/startup/Catalina".equals(className);
    }

    @Override
    public String getType() {
        return "startup";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("start")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    invokeStatic(Type.getType(TomcatStartupHook.class),
                            new Method("checkTomcatStartup", "()V"));
                }
            };
        }
        return mv;
    }

    /**
     * tomcat启动时检测安全规范
     */
    public static void checkTomcatStartup() {
        HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_TOMCAT_START, CheckParameter.EMPTY_MAP);
    }
}
