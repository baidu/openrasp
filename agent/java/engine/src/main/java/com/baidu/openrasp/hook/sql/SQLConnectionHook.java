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

package com.baidu.openrasp.hook.sql;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.baidu.openrasp.plugin.js.engine.JSContextFactory;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.*;
import org.mozilla.javascript.Scriptable;

import java.io.IOException;
import java.sql.SQLException;
import java.util.LinkedList;

/**
 * @description: 检测mysql的access deny异常的hook点
 * @author: anyang
 * @create: 2019/02/14 11:43
 */
@HookAnnotation
public class SQLConnectionHook extends AbstractClassHook {
    private String type;

    @Override
    public boolean isClassMatched(String className) {
        this.type = "mysql";
        return "com/mysql/jdbc/NonRegisteringDriver".equals(className) ||
                "com/mysql/cj/jdbc/NonRegisteringDriver".equals(className);
    }

    @Override
    public String getType() {
        return "sql";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        addCatch(ctClass, "connect", null);
    }


    /**
     * SQL执行异常检测
     *
     * @param server 数据库类型
     * @param e      sql执行抛出的异常
     */
    public static void checkSQLErrorCode(String server, SQLException e) {
        String errorCode = String.valueOf(e.getErrorCode());
        if (errorCode.equals("1045")) {
            JSContext cx = JSContextFactory.enterAndInitContext();
            Scriptable params = cx.newObject(cx.getScope());
            params.put("server", params, server);
            params.put("error_code", params, errorCode);
            params.put("query", params, "");
            String message = server + " error " + e.getErrorCode() + " detected: " + e.getMessage();
            params.put("message", params, message);
            HookHandler.doPolicyCheckWithoutRequest(CheckParameter.Type.SQL, params);
        }
    }

    /**
     * 捕捉hook method抛出的异常
     */
    public void addCatch(CtClass ctClass, String methodName, String desc) throws NotFoundException, CannotCompileException {
        if ("mysql".equals(this.type)) {
            LinkedList<CtBehavior> methods = getMethod(ctClass, methodName, desc);
            if (methods != null && methods.size() > 0) {
                for (CtBehavior method : methods) {
                    if (method != null) {
                        String errorSrc = "com.baidu.openrasp.hook.sql.SQLConnectionHook.checkSQLErrorCode(" + "\"" + type + "\"" + ",$e);";
                        method.addCatch("{" + errorSrc + " throw $e;}", ClassPool.getDefault().get("java.sql.SQLException"));
                    }
                }
            }
        }
    }
}
