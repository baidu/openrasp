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

package com.baidu.openrasp.hook.sql;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.util.HashMap;

/**
 * @description: hibernate hql 检测hook点
 * @author: anyang
 * @create: 2019/05/30 12:02
 */
@HookAnnotation
public class HQLHook extends AbstractClassHook {
    @Override
    public boolean isClassMatched(String className) {
        return "org/hibernate/internal/SessionImpl".equals(className);
    }

    @Override
    public String getType() {
        return "hql";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String src = getInvokeStaticSrc(HQLHook.class, "checkHQL", "$1", String.class);
        insertBefore(ctClass, "createQuery", "(Ljava/lang/String;)Lorg/hibernate/Query;", src);
        insertBefore(ctClass, "createSQLQuery", "(Ljava/lang/String;)Lorg/hibernate/SQLQuery;", src);
    }

    public static void checkHQL(String query) {
        if (!StringUtils.isEmpty(query)) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("query", query);
            params.put("server", "hibernate");
            HookHandler.doCheck(CheckParameter.Type.SQL, params);
        }
    }
}