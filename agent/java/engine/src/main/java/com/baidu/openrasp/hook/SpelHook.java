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

@HookAnnotation
public class SpelHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "spel";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "org/springframework/expression/spel/standard/SpelExpressionParser".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(SpelHook.class, "checkSpelExpression", "$1", String.class);
        insertAfter(ctClass, "doParseExpression", "(Ljava/lang/String;Lorg/springframework/expression/ParserContext;)Lorg/springframework/expression/spel/standard/SpelExpression;", src);
    }

    /**
     * spring框架spel语句解析hook点，解析失败的spel语句不会进入检测点
     *
     * @param object spel语句
     */
    public static void checkSpelExpression(String expression) {
        if (expression != null) {
            if (expression.length() >= Config.getConfig().getSpelMinLength()) {
                HashMap<String, Object> params = new HashMap<String, Object>();
                params.put("expression", expression);
                HookHandler.doCheck(CheckParameter.Type.SPEL, params);
            }
        }
    }

}
