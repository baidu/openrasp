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
import java.io.ObjectStreamClass;

/**
 * Created by tyy on 6/21/17.
 * 反序列化漏洞检测hook
 */
public class DeserializationHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "deserialization";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/io/ObjectInputStream".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(DeserializationHook.class, "checkDeserializationClass",
                "$1", ObjectStreamClass.class);
        insertBefore(ctClass, "resolveClass", "(Ljava/io/ObjectStreamClass;)Ljava/lang/Class;", src);
    }

    /**
     * 反序列化监检测点
     *
     * @param objectStreamClass 反序列化的类的流对象
     */
    public static void checkDeserializationClass(ObjectStreamClass objectStreamClass) {
        if (objectStreamClass != null) {
            String clazz = objectStreamClass.getName();
            if (clazz != null) {
                JSContext cx = JSContextFactory.enterAndInitContext();
                Scriptable params = cx.newObject(cx.getScope());
                params.put("clazz", params, clazz);
                HookHandler.doCheck(CheckParameter.Type.DESERIALIZATION, params);
            }
        }

    }


}
