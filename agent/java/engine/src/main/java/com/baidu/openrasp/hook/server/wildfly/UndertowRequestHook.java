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

import com.baidu.openrasp.hook.server.ServerRequestHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by izpz on 18-10-26.
 * <p>
 * wildfly 请求解析的 hook 点
 */
@HookAnnotation
public class UndertowRequestHook extends ServerRequestHook {
    /**
     * 用于判断类名与当前需要hook的类是否相同
     *
     * @param className 用于匹配的类名
     * @return 是否匹配
     */
    @Override
    public boolean isClassMatched(String className) {
        return "io/undertow/servlet/handlers/ServletInitialHandler".equals(className);
    }

    /**
     * hook 目标类的函数
     *
     * @param ctClass 目标类
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String requestSrc = getInvokeStaticSrc(ServerRequestHook.class, "checkRequest",
                "$0,$1", Object.class, Object.class);
        insertBefore(ctClass, "handleFirstRequest", null, requestSrc);
    }
}
