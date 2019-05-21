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
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.*;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.sql.SQLException;
import java.util.*;

/**
 * @description: 检测mysql的access deny异常的hook点
 * @author: anyang
 * @create: 2019/02/14 11:43
 */
@HookAnnotation
public class SQLConnectionHook extends AbstractClassHook {
    private static final String DEFAULT_MYSQL_PORT = "3306";
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
    public static void checkSQLErrorCode(String server, SQLException e, Object[] object) {
        String errorCode = String.valueOf(e.getErrorCode());
        if (errorCode.equals("1045")) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("server", server);
            params.put("error_code", errorCode);
            String message = server + " error " + e.getErrorCode() + " detected: " + e.getMessage();
            params.put("message", message);
            if (object != null && object.length >= 2) {
                String username = null;
                String host = null;
                String port = null;
                String connectionString = null;
                String socket = null;
                try {
                    String url = (String) object[0];
                    Properties properties = (Properties) object[1];
                    username = properties.getProperty("user");
                    if (url != null && StringUtils.startsWithIgnoreCase(url, "jdbc:mysql://")) {
                        int pos1 = url.indexOf(':', 5);
                        int pos2 = url.indexOf('?', pos1);
                        if (pos2 == -1) {
                            pos2 = url.length();
                        } else {
                            String paramString = url.substring(pos2 + 1);
                            StringTokenizer queryParams = new StringTokenizer(paramString, "&");
                            while (queryParams.hasMoreTokens()) {
                                String parameterValuePair = queryParams.nextToken();
                                int indexOfEquals = parameterValuePair.indexOf("=");
                                if (indexOfEquals > 0) {
                                    if (parameterValuePair.substring(0, indexOfEquals).equals("user")) {
                                        if (username == null) {
                                            username = parameterValuePair.substring(indexOfEquals + 1);
                                        }
                                    } else if (parameterValuePair.substring(0, indexOfEquals).equals("socket")) {
                                        socket = parameterValuePair.substring(indexOfEquals + 1);
                                    }
                                }
                            }
                        }
                        String connUri = url.substring(pos1 + 1, pos2);
                        int pos3 = connUri.indexOf(":");
                        int pos4 = connUri.indexOf('/', 2);
                        if (pos3 != -1) {
                            host = connUri.substring(2, pos3);
                            if (pos4 != -1) {
                                port = connUri.substring(pos3 + 1, pos4);
                            }
                        } else {
                            host = connUri.substring(2, pos4);
                            port = DEFAULT_MYSQL_PORT;
                        }
                        connectionString = url.substring(0, pos2);
                    }
                } catch (Exception e1) {
                    String msg = "parse connection string fail";
                    int code = ErrorType.HOOK_ERROR.getCode();
                    HookHandler.LOGGER.warn(CloudUtils.getExceptionObject(msg, code), e);
                }
                params.put("connectionString", connectionString != null ? connectionString : "");
                params.put("username", username != null ? username : "");
                params.put("hostname", host != null ? host : "");
                params.put("port", port != null ? port : "");
                params.put("socket", socket != null ? socket : "");
            }
            HookHandler.doCheckWithoutRequest(CheckParameter.Type.SQL_EXCEPTION, params);
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
                        String errorSrc = "com.baidu.openrasp.hook.sql.SQLConnectionHook.checkSQLErrorCode(" + "\"" + type + "\"" + ",$e,$args);";
                        method.addCatch("{" + errorSrc + " throw $e;}", ClassPool.getDefault().get("java.sql.SQLException"));
                    }
                }
            }
        }
    }
}
