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
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: websphere stax hook 点
 * @author: anyang
 * @create: 2019/05/24 18:54
 */
@HookAnnotation
public class IBMXmlHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "com/ibm/xml/xlxp2/api/util/SimpleParsedEntityFactory".equals(className);
    }

    @Override
    public String getType() {
        return "xxe";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(IBMXmlHook.class, "checkXXE", "$1", String.class);
        insertBefore(ctClass, "expandSystemID", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", src);
    }

    public static void checkXXE(String expandedSystemId) {
        XXEHook.checkXXE(expandedSystemId);
    }
}
