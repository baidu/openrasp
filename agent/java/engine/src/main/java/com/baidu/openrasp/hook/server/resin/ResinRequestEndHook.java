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

package com.baidu.openrasp.hook.server.resin;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: resin request end hook点
 * @author: anyang
 * @create: 2019/05/31 17:31
 */
@HookAnnotation
public class ResinRequestEndHook extends ServerRequestEndHook {

    @Override
    public boolean isClassMatched(String className) {
        return "com/caucho/server/dispatch/ServletInvocation".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
        // resin3.x
        insertAfter(ctClass, "service",
                "(Ljavax/servlet/ServletRequest;Ljavax/servlet/ServletResponse;)V", requestEndSrc, true);
        // resin4.x
        insertAfter(ctClass, "doResume",
                "(Ljavax/servlet/ServletRequest;Ljavax/servlet/ServletResponse;)Z", requestEndSrc, true);
    }
}
