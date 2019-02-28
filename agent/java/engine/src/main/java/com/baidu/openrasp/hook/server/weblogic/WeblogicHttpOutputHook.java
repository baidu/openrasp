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

package com.baidu.openrasp.hook.server.weblogic;

import com.baidu.openrasp.hook.server.ServerOutputCloseHook;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

/**
 * @description: 插入用户自定义脚本
 * @author: anyang
 * @create: 2018/09/05 13:54
 */
@HookAnnotation
public class WeblogicHttpOutputHook extends ServerOutputCloseHook {
    public static String clazzName = null;

    @Override
    public boolean isClassMatched(String className) {
        if ("weblogic/servlet/internal/ServletOutputStreamImpl".equals(className)) {
            clazzName = className;
            return true;
        }
        return false;
    }

    @Override
    protected void hookMethod(CtClass ctClass, String src) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, "commit", "()V", src);
    }
}
