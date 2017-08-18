package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.Arrays;

/**
 * Created by zhuming01 on 7/18/17.
 * All rights reserved
 */
public class SQLStatementHook extends AbstractClassHook {
    private String type;
    private String[] exceptions;

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
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
            this.type = "mysql";
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/Stmt".equals(className) 
                || "org/sqlite/jdbc3/JDBC3Statement".equals(className)) {
            this.type = "sqlite";
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* Oracle */
        if ("oracle/jdbc/driver/OracleStatement".equals(className)) {
            this.type = "oracle";
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerStatement".equals(className)) {
            this.type = "sqlserver";
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc/PgStatement".equals(className)
                || "org/postgresql/jdbc1/AbstractJdbc1Statement".equals(className)
                || "org/postgresql/jdbc2/AbstractJdbc2Statement".equals(className)
                || "org/postgresql/jdbc3/AbstractJdbc3Statement".equals(className)
                || "org/postgresql/jdbc3/AbstractJdbc3Statement".equals(className)
                || "org/postgresql/jdbc3g/AbstractJdbc3gStatement".equals(className)
                || "org/postgresql/jdbc4/AbstractJdbc4Statement".equals(className)) {
            this.type = "pgsql";
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        return false;
    }

    @Override
    protected MethodVisitor hookMethod(int access, String name, String desc, String signature, String[] exceptions, MethodVisitor mv) {
        boolean hook = false;
        if (name.equals("execute") && Arrays.equals(exceptions, this.exceptions)) {
            if (desc.equals("(Ljava/lang/String;)Z")
                    || desc.equals("(Ljava/lang/String;I)Z")
                    || desc.equals("(Ljava/lang/String;[I)Z")
                    || desc.equals("(Ljava/lang/String;[Ljava/lang/String;)Z")) {
                hook = true;
            }
        } else if (name.equals("executeUpdate") && Arrays.equals(exceptions, this.exceptions)) {
            if (desc.equals("(Ljava/lang/String;)I")
                    || desc.equals("(Ljava/lang/String;I)I")
                    || desc.equals("(Ljava/lang/String;[I)I")
                    || desc.equals("(Ljava/lang/String;[Ljava/lang/String;)I")) {
                hook = true;
            }
        } else if (name.equals("executeQuery") && Arrays.equals(exceptions, this.exceptions)) {
            if (desc.equals("(Ljava/lang/String;)Ljava/sql/ResultSet;")) {
                hook = true;
            }
        } else if (name.equals("addBatch") && Arrays.equals(exceptions, this.exceptions)) {
            if (desc.equals("(Ljava/lang/String;)V")) {
                hook = true;
            }
        }

        return hook ? new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
            @Override
            protected void onMethodEnter() {
                push(type);
                loadArg(0);
                invokeStatic(Type.getType(HookHandler.class),
                        new Method("checkSQL", "(Ljava/lang/String;Ljava/lang/String;)V"));
            }
        } : mv;
    }
}
