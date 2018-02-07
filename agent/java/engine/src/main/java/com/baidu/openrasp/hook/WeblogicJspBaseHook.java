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

package com.baidu.openrasp.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.catalina.ApplicationFilterHook;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by zhuming01 on 6/28/17.
 * All rights reserved
 */
public class WeblogicJspBaseHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "request";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "weblogic/servlet/jsp/JspBase".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String hookDesc = "(Ljavax/servlet/ServletRequest;Ljavax/servlet/ServletResponse;)V";
        String srcBefore = getInvokeStaticSrc(ApplicationFilterHook.class, "checkRequest",
                "$0,$1,$2", Object.class, Object.class, Object.class);
        insertBefore(ctClass, "service", hookDesc, srcBefore);
        String srcAfter = getInvokeStaticSrc(HookHandler.class, "onServiceExit", "");
        insertAfter(ctClass, "service", hookDesc, srcAfter, true);
    }

}
