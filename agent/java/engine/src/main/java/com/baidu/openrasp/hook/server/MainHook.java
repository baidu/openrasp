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

package com.baidu.openrasp.hook.server;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.server.ServerXssHook;
import com.baidu.openrasp.hook.server.websphere.WebsphereXssHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import com.baidu.openrasp.tool.hook.ServerXss;
import com.baidu.openrasp.tool.model.ApplicationModel;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.HashMap;

/**
 * @author anyang
 * @Description: websphere的xss检测hook点
 * @date 2018/8/15 14:18
 */
@HookAnnotation
public class MainHook extends ServerXssHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/jboss/Main".equals(className);
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(MainHook.class, "boot", "$0", Object.class);
        insertBefore(ctClass, "boot", "([Ljava/lang/String;)V", src);
    }

    public static void boot(Object object) {
        System.out.println("{}{}{}{}{}{}{}{}{");
        System.out.println("{}{}{}{}{}{}{}{}{");
        System.out.println("{}{}{}{}{}{}{}{}{");
        System.out.println("{}{}{}{}{}{}{}{}{");
    }
}
