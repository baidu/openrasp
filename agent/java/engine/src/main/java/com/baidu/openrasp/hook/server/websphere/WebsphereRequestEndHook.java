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

package com.baidu.openrasp.hook.server.websphere;

import com.baidu.openrasp.hook.server.ServerRequestEndHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: websphere request end hook点
 * @author: anyang
 * @create: 2019/05/31 17:35
 */
@HookAnnotation
public class WebsphereRequestEndHook extends ServerRequestEndHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/ws/webcontainer/filter/WebAppFilterManager".equals(className)
                || "com/ibm/ws/webcontainer/webapp/WebApp".equals(className)
                || "com/ibm/ws/webcontainer/servlet/CacheServletWrapper".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        if (ctClass.getName().contains("WebAppFilterManager")) {
            String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
            insertAfter(ctClass, "invokeFilters", null, requestEndSrc, true);
        } else {
            String requestEndSrc = getInvokeStaticSrc(ServerRequestEndHook.class, "checkRequestEnd", "");
            insertAfter(ctClass, "handleRequest", null, requestEndSrc, true);
        }
    }
}
