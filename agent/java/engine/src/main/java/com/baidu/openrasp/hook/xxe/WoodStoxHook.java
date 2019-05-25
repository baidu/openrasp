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

package com.baidu.openrasp.hook.xxe;

import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;


import java.io.IOException;

/**
 * @description: woodStox解析xxe
 * @author: anyang
 * @create: 2019/05/16 15:53
 */
@HookAnnotation
public class WoodStoxHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ctc/wstx/sr/StreamScanner".equals(className);
    }

    @Override
    public String getType() {
        return "xxe";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(WoodStoxHook.class, "checkXXE", "$1", Object.class);
        insertBefore(ctClass, "expandEntity", "(Lcom/ctc/wstx/ent/EntityDecl;Z)V", src);
    }

    public static void checkXXE(Object entity) {
        if (entity != null) {
            Boolean isExternal = (Boolean) Reflection.invokeMethod(entity, "isExternal", new Class[]{});
            if (isExternal != null && isExternal) {
                String expandedSystemId = Reflection.invokeStringMethod(entity, "getSystemId", new Class[]{});
                XXEHook.checkXXE(expandedSystemId);
            }
        }
    }
}
