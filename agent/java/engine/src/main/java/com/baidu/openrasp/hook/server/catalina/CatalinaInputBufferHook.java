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

package com.baidu.openrasp.hook.server.catalina;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.hook.server.ServerInputHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by zhuming01 on 7/5/17.
 * All rights reserved
 */
public class CatalinaInputBufferHook extends ServerInputHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/apache/catalina/connector/InputBuffer".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "($w)$_,$0", int.class, Object.class);
        insertAfter(ctClass, "readByte", "()I", readByteSrc);
        String readSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "($w)$_,$0,$1,($w)$2,($w)$3", int.class, Object.class, byte[].class, int.class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);
    }

}
