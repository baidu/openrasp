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
            this.type = SQL_TYPE_MYSQL;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQLite */
        if ("org/sqlite/Conn".equals(className)
                || "org/sqlite/jdbc4/JDBC4Connection".equals(className)) {
            this.type = SQL_TYPE_SQLITE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* Oracle */
        if ("oracle/jdbc/driver/PhysicalConnection".equals(className)) {
            this.type = SQL_TYPE_ORACLE;
            this.exceptions = new String[]{"java/sql/SQLException"};
            return true;
        }

        /* SQL Server */
        if ("com/microsoft/sqlserver/jdbc/SQLServerConnection".equals(className)) {
            this.type = SQL_TYPE_SQLSERVER;
            this.exceptions = new String[]{"com/microsoft/sqlserver/jdbc/SQLServerException"};
            return true;
        }

        /* PostgreSQL */
        if ("org/postgresql/jdbc3/Jdbc3Connection".equals(className)
                || "org/postgresql/jdbc4/Jdbc4Connection".equals(className)) {
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

         /* HSqlDB */
        if ("org/hsqldb/jdbc/JDBCConnection".equals(className)) {
            this.type = SQL_TYPE_HSQL;
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
        if (this.type.equals(SQL_TYPE_MYSQL)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_SQLITE)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;III)Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_ORACLE)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_SQLSERVER)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;IILcom/microsoft/sqlserver/jdbc/SQLServerStatementColumnEncryptionSetting;)" +
                            "Ljava/sql/PreparedStatement;", checkSqlSrc);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;IIILcom/microsoft/sqlserver/jdbc/SQLServerStatementColumnEncryptionSetting;)" +
                            "Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_PGSQL)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement",
                    "(Ljava/lang/String;III)Ljava/sql/PreparedStatement;", checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_DB2)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement", null, checkSqlSrc);
        } else if (this.type.equals(SQL_TYPE_HSQL)) {
            String checkSqlSrc = getInvokeStaticSrc(SQLStatementHook.class, "checkSQL",
                    "\"" + type + "\"" + ",$0,$1", String.class, Object.class, String.class);
            insertBeforeAndAddCatch(ctClass, "prepareStatement", null, checkSqlSrc);
        }
    }

    private void insertBeforeAndAddCatch(CtClass ctClass, String methodName, String desc, String insertSrc) throws NotFoundException, CannotCompileException {
        insertBefore(ctClass, methodName, desc, insertSrc);
        addCatch(ctClass, methodName, new String[]{desc});
    }
}
