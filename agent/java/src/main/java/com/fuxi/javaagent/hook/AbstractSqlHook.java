package com.fuxi.javaagent.hook;

/**
 * Created by tyy on 17-11-6.
 * sql相关hook点的基类
 */
public abstract class AbstractSqlHook extends AbstractClassHook {

    public static final String SQL_TYPE_MYSQL = "mysql";
    public static final String SQL_TYPE_SQLITE = "sqlite";
    public static final String SQL_TYPE_ORACLE = "oracle";
    public static final String SQL_TYPE_SQLSERVER = "sqlserver";
    public static final String SQL_TYPE_PGSQL = "pgsql";
    public static final String SQL_TYPE_DB2 = "db2";

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
}
