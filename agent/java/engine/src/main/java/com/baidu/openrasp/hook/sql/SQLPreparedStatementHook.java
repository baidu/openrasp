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

package com.baidu.openrasp.hook.sql;

import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 18-4-28.
 *
 * sql Prepare 查询 hook 点
 */
public class SQLPreparedStatementHook extends AbstractSqlHook {

    private String className;

    @Override
    public boolean isClassMatched(String className) {

        /* MySQL */
        if ("com/mysql/jdbc/PreparedStatement".equals(className)
                || "com/mysql/cj/jdbc/PreparedStatement".equals(className)) {
            this.type = SQL_TYPE_MYSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/PrepStmt".equals(className)
                || "org/sqlite/jdbc3/JDBC3PreparedStatement".equals(className)) {
            this.type = SQL_TYPE_SQLITE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* Oracle */
        if ("oracle/jdbc/driver/OraclePreparedStatement".equals(className)) {
            this.type = SQL_TYPE_ORACLE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerPreparedStatement".equals(className)) {
            this.type = SQL_TYPE_SQLSERVER;
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc/PgPreparedStatement".equals(className)
                || "org/postgresql/jdbc1/AbstractJdbc1Statement".equals(className)
                || "org/postgresql/jdbc2/AbstractJdbc2Statement".equals(className)
                || "org/postgresql/jdbc3/AbstractJdbc3Statement".equals(className)
                || "org/postgresql/jdbc3g/AbstractJdbc3gStatement".equals(className)
                || "org/postgresql/jdbc4/AbstractJdbc4Statement".equals(className)) {
            this.className = className;
            this.type = SQL_TYPE_PGSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* DB2 */
        if ("com/ibm/db2/jcc/am/Connection".equals(className)) {
            this.type = SQL_TYPE_DB2;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        return false;
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "sql_prepared";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        hookSqlPreparedStatementMethod(ctClass);
    }

    private void hookSqlPreparedStatementMethod(CtClass ctClass) throws NotFoundException, CannotCompileException {
        String originalSqlCode = null;
        String checkSqlSrc = null;
        if (SQL_TYPE_MYSQL.equals(this.type)) {
            originalSqlCode = "originalSql";
        } else if (SQL_TYPE_SQLITE.equals(this.type)) {
            originalSqlCode = "this.sql";
        } else if (SQL_TYPE_SQLSERVER.equals(this.type)) {
            originalSqlCode = "preparedSQL";
        } else if (SQL_TYPE_PGSQL.equals(this.type)) {
            if ("org/postgresql/jdbc/PgPreparedStatement".equals(className)) {
                originalSqlCode = "preparedQuery.query.toString(preparedQuery.query.createParameterList())";
            } else {
                originalSqlCode = "preparedQuery.toString(preparedQuery.createParameterList())";
            }
        } else if (SQL_TYPE_ORACLE.equals(this.type)) {
            originalSqlCode = "this.sqlObject.getOriginalSql()";
        }
        if (originalSqlCode != null) {
            checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0," + originalSqlCode, String.class, Object.class, String.class);
            insertBefore(ctClass, "execute", "()Z", checkSqlSrc);
            insertBefore(ctClass, "executeUpdate", "()I", checkSqlSrc);
            insertBefore(ctClass, "executeQuery", "()Ljava/sql/ResultSet;", checkSqlSrc);
            try {
                insertBefore(ctClass, "executeBatch", "()[I", checkSqlSrc);
            } catch (CannotCompileException e) {
                insertBefore(ctClass, "executeBatchInternal", null, checkSqlSrc);
            }
        } else if (SQL_TYPE_DB2.equals(this.type)) {
            checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBefore(ctClass, "prepareStatement", null, checkSqlSrc);
        }
    }

}
