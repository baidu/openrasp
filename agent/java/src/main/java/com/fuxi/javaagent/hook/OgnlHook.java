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
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

/**
 * Created by tyy on 6/21/17.
 */
public class OgnlHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "ognl";
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "ognl/Ognl".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc,
                                       String signature, String[] exceptions, MethodVisitor mv) {
        if ("parseExpression".equals(name) && "(Ljava/lang/String;)Ljava/lang/Object;".equals(desc)) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(0);
                    invokeStatic(Type.getType(OgnlHook.class),
                            new Method("checkOgnlExpression", "(Ljava/lang/String;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * struct框架ognl语句解析hook点
     *
     * @param expression ognl语句
     */
    public static void checkOgnlExpression(String expression) {
        if (expression != null) {
            if (expression.length() >= Config.getConfig().getOgnlMinLength()) {
                JSContext cx = JSContextFactory.enterAndInitContext();
                Scriptable params = cx.newObject(cx.getScope());
                params.put("expression", params, expression);
                HookHandler.doCheck(CheckParameter.Type.OGNL, params);
            }
        }
    }

}
