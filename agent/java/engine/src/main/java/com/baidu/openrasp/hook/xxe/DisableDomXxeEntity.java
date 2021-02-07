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

package com.baidu.openrasp.hook.xxe;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * @description: 关闭DocumentBuilderFactory的XXE entity
 * @author: anyang
 * @create: 2019/04/30 14:35
 */
@HookAnnotation
public class DisableDomXxeEntity extends DisableXxeEntity {
    @Override
    public boolean isClassMatched(String className) {
        return "com/sun/org/apache/xerces/internal/parsers/DOMParser".equals(className) ||
                "org/apache/xerces/parsers/DOMParser".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(DisableDomXxeEntity.class, "setFeature", "$0", Object.class);
        insertBefore(ctClass, "parse", null, src);
    }

    public static void setFeature(Object parser) {
        if (HookHandler.isEnableCurrThreadHook()) {
            String action = getAction();
            if (BLOCK_XXE_DISABLE_ENTITY.equals(action) && getStatus("java_dom")) {
                try {
                    Reflection.invokeMethod(parser, "setFeature",
                            new Class[]{String.class, boolean.class}, FEATURE, true);
                } catch (Throwable t) {
                    LogTool.traceHookWarn("Dom close xxe entity failed: " + t.getMessage(), t);
                }
            }
        }

    }
}
