package com.baidu.openrasp.hook.sql;

import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;

/**
 * Created by tyy on 19-9-30.
 */
@HookAnnotation
public class SqlConnectionPreparedHook extends AbstractSqlHook {

    @Override
    public boolean isClassMatched(String className) {
        /* MySQL */
        if ("com/mysql/jdbc/ConnectionImpl".equals(className)
                || "com/mysql/cj/jdbc/ConnectionImpl".equals(className)) {
            this.type = SqlType.MYSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/Conn".equals(className)
                || "org/sqlite/jdbc4/JDBC4Connection".equals(className)) {
            this.type = SqlType.SQLITE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* Oracle */
        if ("oracle/jdbc/driver/PhysicalConnection".equals(className)) {
            this.type = SqlType.ORACLE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerConnection".equals(className)) {
            this.type = SqlType.SQLSERVER;
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc/PgConnection".equals(className)
                || "org/postgresql/jdbc3/Jdbc3Connection".equals(className)
                || "org/postgresql/jdbc4/Jdbc4Connection".equals(className)) {
            this.type = SqlType.PGSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* DB2 */
        if ("com/ibm/db2/jcc/am/Connection".equals(className)) {
            this.type = SqlType.DB2;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

         /* HSqlDB */
        if ("org/hsqldb/jdbc/JDBCConnection".equals(className)
                || "org/hsqldb/jdbc/jdbcConnection".equals(className)) {
            this.type = SqlType.HSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        return false;
    }

    @Override
    public String getType() {
        return "sqlPrepared";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                "\"" + type.name + "\"" + ",$0,$1", String.class, Object.class, String.class);
        if (this.type.equals(SqlType.MYSQL)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall",
                    "(Ljava/lang/String;II)Ljava/sql/CallableStatement;", checkSqlSrc);
        } else if (this.type.equals(SqlType.SQLITE)) {
            // SQLite does not support Stored Procedures
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;III)Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SqlType.ORACLE)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall",
                    "(Ljava/lang/String;II)Ljava/sql/CallableStatement;", checkSqlSrc);
        } else if (this.type.equals(SqlType.SQLSERVER)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;IILcom/microsoft/sqlserver/jdbc/SQLServerStatementColumnEncryptionSetting;)" +
                            "Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;IIILcom/microsoft/sqlserver/jdbc/SQLServerStatementColumnEncryptionSetting;)" +
                            "Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall", "(Ljava/lang/String;II)" +
                    "Ljava/sql/CallableStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall",
                    "(Ljava/lang/String;IIILcom/microsoft/sqlserver/jdbc/SQLServerStatementColumnEncryptionSetting;)" +
                            "Ljava/sql/CallableStatement;", checkSqlSrc);
        } else if (this.type.equals(SqlType.PGSQL)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;III)Ljava/sql/PreparedStatement;", checkSqlSrc);
//            insertBeforeAndAddCatch(ctClass, "prepareStatement",
//                    "(Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall",
                    "(Ljava/lang/String;III)Ljava/sql/CallableStatement;", checkSqlSrc);
        } else if (this.type.equals(SqlType.DB2)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement", null, checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall", null, checkSqlSrc);
        } else if (this.type.equals(SqlType.HSQL)) {
            insertBeforeAndAddCatch(ctClass, "prepareStatement", null, checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareCall", null, checkSqlSrc);
        }
    }

    private void insertBeforeAndAddCatch(CtClass ctClass, String methodName, String desc, String insertSrc) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, methodName, desc, insertSrc);
        addCatch(ctClass, methodName, new String[]{desc});
    }
}
