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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.sql.SQLException;
import java.util.HashMap;

/**
 * Created by zhuming01 on 7/18/17.
 * All rights reserved
 */
@HookAnnotation
public class SQLStatementHook extends AbstractSqlHook {

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "sql";
    }

    @Override
    public boolean isClassMatched(String className) {
        /* MySQL */
        if ("com/mysql/jdbc/StatementImpl".equals(className)
                || "com/mysql/cj/jdbc/StatementImpl".equals(className)) {
            this.type = SQL_TYPE_MYSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/Stmt".equals(className)
                || "org/sqlite/jdbc3/JDBC3Statement".equals(className)) {
            this.type = SQL_TYPE_SQLITE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* Oracle */
        if ("oracle/jdbc/driver/OracleStatement".equals(className)) {
            this.type = SQL_TYPE_ORACLE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerStatement".equals(className)) {
            this.type = SQL_TYPE_SQLSERVER;
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc/PgStatement".equals(className)
                || "org/postgresql/jdbc1/AbstractJdbc1Statement".equals(className)
                || "org/postgresql/jdbc2/AbstractJdbc2Statement".equals(className)
                || "org/postgresql/jdbc3/AbstractJdbc3Statement".equals(className)
                || "org/postgresql/jdbc3g/AbstractJdbc3gStatement".equals(className)
                || "org/postgresql/jdbc4/AbstractJdbc4Statement".equals(className)) {
            this.type = SQL_TYPE_PGSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* DB2 */
        if (className != null && className.startsWith("com/ibm/db2/jcc/am")) {
            this.type = SQL_TYPE_DB2;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        return false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        CtClass[] interfaces = ctClass.getInterfaces();
        if (SQL_TYPE_DB2.equals(this.type) && interfaces != null) {
            for (CtClass inter : interfaces) {
                if ("com.ibm.db2.jcc.DB2Statement".equals(inter.getName())) {
                    if (interfaces.length > 2) {
                        hookSqlStatementMethod(ctClass);
                    }
                }
            }
        } else {
            hookSqlStatementMethod(ctClass);
        }
    }

    private void hookSqlStatementMethod(CtClass ctClass) throws NotFoundException, CannotCompileException {
        String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
        insertBefore(ctClass, "execute", checkSqlSrc,
                new String[]{"(Ljava/lang/String;)Z", "(Ljava/lang/String;I)Z",
                        "(Ljava/lang/String;[I)Z", "(Ljava/lang/String;[Ljava/lang/String;)Z"});
        insertBefore(ctClass, "executeUpdate", checkSqlSrc,
                new String[]{"(Ljava/lang/String;)I", "(Ljava/lang/String;I)I",
                        "(Ljava/lang/String;[I)I", "(Ljava/lang/String;[Ljava/lang/String;)I"});
        insertBefore(ctClass, "executeQuery",
                "(Ljava/lang/String;)Ljava/sql/ResultSet;", checkSqlSrc);
        insertBefore(ctClass, "addBatch",
                "(Ljava/lang/String;)V", checkSqlSrc);
        addCatch(ctClass, "execute", null);
        addCatch(ctClass, "executeUpdate", null);
        addCatch(ctClass, "executeQuery", null);
        addCatch(ctClass, "addBatch", null);
    }

    /**
     * SQL语句检测
     *
     * @param stmt sql语句
     */
    public static void checkSQL(String server, Object statement, String stmt) {
        if (stmt != null && !stmt.isEmpty()) {
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("server", server);
            params.put("query", stmt);
            HookHandler.doCheck(CheckParameter.Type.SQL, params);
        }
    }

    /**
     * SQL执行异常检测
     *
     * @param server 数据库类型
     * @param e      sql执行抛出的异常
     */
    public static void checkSQLErrorCode(String server, SQLException e, Object[] object) {
        if (object != null && object.length > 0 && !StringUtils.isEmpty((String) object[0])) {
            if (checkSqlErrorCode(e)) {
                return;
            }
            HashMap<String, Object> params = new HashMap<String, Object>();
            params.put("server", server);
            params.put("query", String.valueOf(object[0]));
            params.put("error_code", String.valueOf(e.getErrorCode()));
            String message = server + " error " + e.getErrorCode() + " detected: " + e.getMessage();
            params.put("message", message);
            HookHandler.doCheck(CheckParameter.Type.SQL_EXCEPTION, params);
        }
    }
}
