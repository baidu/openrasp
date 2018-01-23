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

import com.baidu.openrasp.hook.AbstractClassHook;

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
