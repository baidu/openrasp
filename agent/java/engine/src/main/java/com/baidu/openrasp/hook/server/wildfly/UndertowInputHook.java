/*
 * Copyright 2017-2019 Baidu Inc.
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
package com.baidu.openrasp.hook.server.wildfly;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerInputHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by izpz on 18-10-26.
 * <p>
 * 获取undertow body
 */
@HookAnnotation
public class UndertowInputHook extends ServerInputHook {

    /**
     * 用于判断类名与当前需要hook的类是否相同
     *
     * @param className 用于匹配的类名
     * @return 是否匹配
     */
    @Override
    public boolean isClassMatched(String className) {
        return "io/undertow/servlet/spec/ServletInputStreamImpl".equals(className);
    }

    /**
     * hook 目标类的函数
     *
     * @param ctClass 目标类
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String readByteSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0", int.class, Object.class);
        insertAfter(ctClass, "read", "()I", readByteSrc);

        String readSrc = getInvokeStaticSrc(HookHandler.class, "onInputStreamRead",
                "$_,$0,$1,$2,$3", int.class, Object.class, byte[].class, int.class, int.class);
        insertAfter(ctClass, "read", "([BII)I", readSrc);
    }
}
