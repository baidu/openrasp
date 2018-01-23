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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.sql.ResultSet;
import java.util.Arrays;
import java.util.HashMap;

/**
 * Created by tyy on 17-11-6.
 * 为检测慢查询添加便利 sql 查询结果的 hook 点
 */
public class SQLResultSetHook extends AbstractSqlHook {

    @Override
    public boolean isClassMatched(String className) {
         /* MySQL */
        if ("com/mysql/jdbc/ResultSetImpl".equals(className)
                || "com/mysql/cj/jdbc/result/ResultSetImpl".equals(className)) {
            this.type = SQL_TYPE_MYSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/RS".equals(className)
                || "org/sqlite/jdbc3/JDBC3ResultSet".equals(className)) {
            this.type = SQL_TYPE_SQLITE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

       /* Oracle */
        if ("oracle/jdbc/driver/OracleResultSetImpl".equals(className)) {
            this.type = SQL_TYPE_ORACLE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerResultSet".equals(className)) {
            this.type = SQL_TYPE_SQLSERVER;
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc/PgResultSet".equals(className)
                || "org/postgresql/jdbc1/AbstractJdbc1ResultSet".equals(className)
                || "org/postgresql/jdbc2/AbstractJdbc2ResultSet".equals(className)
                || "org/postgresql/jdbc3/AbstractJdbc3ResultSet".equals(className)
                || "org/postgresql/jdbc3g/AbstractJdbc3gResultSet".equals(className)
                || "org/postgresql/jdbc4/AbstractJdbc4ResultSet".equals(className)) {
            this.type = SQL_TYPE_PGSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        return false;
    }

    @Override
    public String getType() {
        return "sql_result_set";
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("next") && desc.equals("()Z") && Arrays.equals(exceptions, this.exceptions)) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    push(type);
                    loadThis();
                    invokeStatic(Type.getType(SQLResultSetHook.class),
                            new Method("checkSqlQueryResult", "(Ljava/lang/String;Ljava/lang/Object;)V"));
                }
            };
        }
        return mv;
    }

    /**
     * 检测数据库查询结果
     *
     * @param sqlResultSet 数据库查询结果
     */
    public static void checkSqlQueryResult(String server, Object sqlResultSet) {
        HashMap<String, Object> params = null;
        try {
            ResultSet resultSet = (ResultSet) sqlResultSet;
            int queryCount = resultSet.getRow();
            params = new HashMap<String, Object>(4);
            params.put("query_count", queryCount);
            params.put("server", server);
        } catch (Exception e) {
            e.printStackTrace();
        }
        HookHandler.doCheck(CheckParameter.Type.SQL_SLOW_QUERY, params);
    }

}
