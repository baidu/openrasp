package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.Arrays;

/**
 * Created by tyy on 17-11-6.
 * 为检测慢查询添加便利 sql 查询结果的 hook 点
 */
public class SqlResultSetHook extends AbstractSqlHook {

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
                    invokeStatic(Type.getType(HookHandler.class),
                            new Method("checkSqlQueryResult", "(Ljava/lang/String;Ljava/lang/Object;)V"));
                }
            };
        }
        return mv;
    }
}
