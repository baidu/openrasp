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
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by lanyuhang on 10/19/17.
 * All rights reserved
 */
public class JstlImportHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "include";
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/taglibs/standard/tag/common/core/ImportSupport".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("targetUrl")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodExit(int opcode) {
                    if (opcode != Opcodes.ATHROW) {
                        mv.visitInsn(Opcodes.DUP);
                        invokeStatic(Type.getType(JstlImportHook.class),
                                new Method("checkJstlImport", "(Ljava/lang/String;)V"));
                    }
                }
            };
        }
        return mv;
    }

    /**
     * 检测 c:import
     *
     * @param url
     */
    public static void checkJstlImport(String url) {
        if (url != null && !url.startsWith("/") && url.contains("://")) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("url", params, url);
            params.put("function", params, "jstl_import");
            HookHandler.doCheck(CheckParameter.Type.INCLUDE, params);
        }
    }
}
