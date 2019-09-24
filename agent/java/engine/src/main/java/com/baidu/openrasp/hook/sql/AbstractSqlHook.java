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
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.hook.AbstractClassHook;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import javassist.*;
import org.apache.commons.lang3.StringUtils;

import java.sql.SQLException;
import java.util.HashMap;
import java.util.LinkedList;

/**
 * Created by tyy on 17-11-6.
 * <p>
 * sql相关hook点的基类
 */
public abstract class AbstractSqlHook extends AbstractClassHook {

    static final String SQL_TYPE_MYSQL = "mysql";
    static final String SQL_TYPE_SQLITE = "sqlite";
    static final String SQL_TYPE_ORACLE = "oracle";
    static final String SQL_TYPE_SQLSERVER = "sqlserver";
    static final String SQL_TYPE_PGSQL = "pgsql";
    static final String SQL_TYPE_DB2 = "db2";
    static final String SQL_TYPE_HSQL = "hsql";

    protected String type;
    protected String[] exceptions;

    public void setType(String type) {
        this.type = type;
    }

    public String[] getExceptions() {
        return exceptions;
    }

    public void setExceptions(String[] exceptions) {
        this.exceptions = exceptions;
    }

    /**
     * 捕捉sql statement抛出的异常
     */
    public void addCatch(CtClass ctClass, String methodName, String[] descs) throws NotFoundException, CannotCompileException {
        //目前只支持对mysql的执行异常检测
        if ("mysql".equals(type)) {
            for (String desc : descs) {
                LinkedList<CtBehavior> methods = getMethod(ctClass, methodName, desc);
                if (methods != null && methods.size() > 0) {
                    for (CtBehavior method : methods) {
                        try {
                            String errorSrc = "com.baidu.openrasp.hook.sql.AbstractSqlHook.checkSQLErrorCode(" + "\"" + type + "\"" + ",$e,$1);";
                            method.addCatch("{" + errorSrc + " throw $e;}", ClassPool.getDefault().get("java.sql.SQLException"));
                            HookHandler.LOGGER.info("add catch to method: " + method.getLongName());
                        } catch (Throwable t) {
                            LogTool.error(ErrorType.HOOK_ERROR, "failed to add catch to method: " + method.getLongName(), t);
                        }
                    }
                }
            }
        }
    }

    /**
     * 捕捉sql preparedStatement抛出的异常
     */
    public void addCatch(CtClass ctClass, String methodName, String desc, String query) throws NotFoundException, CannotCompileException {
        //目前只支持对mysql的执行异常检测
        if ("mysql".equals(type)) {
            LinkedList<CtBehavior> methods = getMethod(ctClass, methodName, desc);
            if (methods != null && methods.size() > 0) {
                for (CtBehavior method : methods) {
                    if (method != null) {
                        String errorSrc = "com.baidu.openrasp.hook.sql.AbstractSqlHook.checkSQLErrorCode(" + "\"" + type + "\"" + ",$e," + query + ");";
                        method.addCatch("{" + errorSrc + " throw $e;}", ClassPool.getDefault().get("java.sql.SQLException"));
                    }
                }
            }
        }
    }

    /**
     * 检测 error code 合法性
     */
    public static boolean checkSqlException(SQLException e) {
        int errCode = e.getErrorCode();
        if (e != null && errCode == 0) {
            String message = "Unable to derive error code from SQL exceptions, error message: " + e.getMessage() + "." +
                    "Please refer to https://rasp.baidu.com/doc/usage/exception.html#faq-errorcode for details.";
            LogTool.traceHookWarn(message, e);
            return true;
        }
        return false;
    }

    /**
     * SQL执行异常检测
     *
     * @param server 数据库类型
     * @param e      sql执行抛出的异常
     * @param query  sql语句
     */
    public static void checkSQLErrorCode(String server, SQLException e, String query) {
        if (Config.getConfig().getSqlErrorCodes().contains(e.getErrorCode())
                && !StringUtils.isEmpty(query) && !checkSqlException(e)) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("server", server);
            params.put("query", query);
            params.put("error_code", e.getErrorCode());
            String message = server + " error " + e.getErrorCode() + " detected: " + e.getMessage();
            params.put("error_msg", message);
            HookHandler.doCheck(CheckParameter.Type.SQL_EXCEPTION, params);
        }
    }
}
