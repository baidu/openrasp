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
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.mozilla.javascript.Scriptable;

import java.io.IOException;
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
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "xxe";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "com/sun/org/apache/xerces/internal/util/XMLResourceIdentifierImpl".equals(className)
                || "org/apache/xerces/util/XMLResourceIdentifierImpl".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(XXEHook.class, "checkXXE", "$4", String.class);
        insertBefore(ctClass, "setValues",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", src);
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
