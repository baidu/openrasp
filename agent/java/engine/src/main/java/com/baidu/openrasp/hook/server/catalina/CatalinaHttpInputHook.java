/*
 * Copyright 2017-2021 Baidu Inc.
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

package com.baidu.openrasp.hook.server.catalina;

import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by zhuming01 on 7/5/17.
 * All rights reserved
 */
@HookAnnotation
public class CatalinaHttpInputHook extends ServerInputHook {

    private String className;

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        if ("org/apache/catalina/connector/InputBuffer".equals(className)
                || "org/apache/catalina/connector/CoyoteReader".equals(className)) {
            this.className = className;
            return true;
        }
        return false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        if (className.equals("org/apache/catalina/connector/InputBuffer")) {
            //2021.7.1 排查，ok
            String readByteSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "readByte", "()I", readByteSrc);
            String readSrc = getInvokeStaticSrc(ServerInputHook.class, "onInputStreamRead",
                    "$_,$0,$1,$2", int.class, Object.class, byte[].class, int.class);
            insertAfter(ctClass, "read", "([BII)I", readSrc);
        } else {
            String src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead", "$_,$0", int.class, Object.class);
            insertAfter(ctClass, "read", "()I", src);
            //2021.7.1修改错误
            src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0,$1", int.class, Object.class, char[].class);
            insertAfter(ctClass, "read", "([C)I", src);

            src = getInvokeStaticSrc(ServerInputHook.class, "onCharRead",
                    "$_,$0,$1,$2", int.class, Object.class, char[].class, int.class);
            insertAfter(ctClass, "read", "([CII)I", src);
        }
    }

}
