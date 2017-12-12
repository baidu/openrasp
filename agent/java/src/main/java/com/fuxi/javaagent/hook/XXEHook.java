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
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.js.engine.JSContext;
import com.fuxi.javaagent.plugin.js.engine.JSContextFactory;
import org.mozilla.javascript.Scriptable;
import org.objectweb.asm.Handle;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.HashSet;

/**
 * Created by lxk on 6/1/17.
 * XXE漏洞检测hook点
 */
public class XXEHook extends AbstractClassHook {

    private static ThreadLocal<HashSet<String>> localExpandedSystemIds = new ThreadLocal<HashSet<String>>() {
        @Override
        protected HashSet<String> initialValue() {
            return new HashSet<String>();
        }
    };

    /**
     * @return 当前线程已触发检测的expandedSystemIds
     */
    public static HashSet<String> getLocalExpandedSystemIds() {
        return localExpandedSystemIds.get();
    }

    /**
     * 重置当前线程已触发检测的expandedSystemIds
     */
    public static void resetLocalExpandedSystemIds() {
        localExpandedSystemIds.get().clear();
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "xxe";
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/sun/org/apache/xerces/internal/util/XMLResourceIdentifierImpl".equals(className)
                || "org/apache/xerces/util/XMLResourceIdentifierImpl".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("setValues") && desc.startsWith("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
                + "Ljava/lang/String;)")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(3);
                    invokeStatic(Type.getType(XXEHook.class),
                            new Method("checkXXE", "(Ljava/lang/String;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * xml语句解析hook点
     *
     * @param expandedSystemId
     */
    public static void checkXXE(String expandedSystemId) {
        if (expandedSystemId != null && !XXEHook.getLocalExpandedSystemIds().contains(expandedSystemId)) {
            XXEHook.getLocalExpandedSystemIds().add(expandedSystemId);
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("entity", params, expandedSystemId);
            HookHandler.doCheck(CheckParameter.Type.XXE, params);
        }
    }
}
