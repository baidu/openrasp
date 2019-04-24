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

package com.baidu.openrasp.hook;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import java.util.HashMap;

import java.io.IOException;

/**
 * Created by tyy on 6/21/17.
 */
@HookAnnotation
public class OgnlHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "ognl";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "ognl/OgnlParser".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(OgnlHook.class, "checkOgnlExpression",
                "$_", Object.class);
        insertAfter(ctClass, "topLevelExpression", null, src);
    }

    /**
     * struct框架ognl语句解析hook点
     *
     * @param object ognl语句
     */
    public static void checkOgnlExpression(Object object) {


        if (object != null) {
            String expression = String.valueOf(object);
            if (expression.length() >= Config.getConfig().getOgnlMinLength()) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                params.put("expression", expression);
                HookHandler.doCheck(CheckParameter.Type.OGNL, params);
            }
        }
    }

}
