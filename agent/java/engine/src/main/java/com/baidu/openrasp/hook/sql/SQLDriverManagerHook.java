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
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.policy.SqlConnectionChecker;
import com.baidu.openrasp.tool.TimeUtils;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.sql.Connection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Properties;

/**
 * Created by lxk on 7/6/17.
 * All rights reserved
 */
@HookAnnotation
public class SQLDriverManagerHook extends AbstractClassHook {
    private static ArrayList<String> classList = new ArrayList<String>();

    static {
        classList.add("com/mysql/jdbc/NonRegisteringDriver");
        classList.add("com/mysql/cj/jdbc/NonRegisteringDriver");
        classList.add("org/sqlite/JDBC");
        classList.add("com/microsoft/sqlserver/jdbc/SQLServerDriver");
        classList.add("org/postgresql/Driver");
        classList.add("oracle/jdbc/driver/OracleDriver");
        classList.add("com/ibm/db2/jcc/DB2Driver");
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "sql";
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return classList.contains(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.baidu.openrasp.hook.AbstractClassHook#hookMethod(CtClass)
     */
    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String srcBefore = getInvokeStaticSrc(SQLDriverManagerHook.class, "checkSqlConnectionOnEnter",
                "$1,$2", String.class, Properties.class);
        String srcAfter = getInvokeStaticSrc(SQLDriverManagerHook.class, "checkSqlConnectionOnExit",
                "$_,$1,$2", Connection.class, String.class, Properties.class);
        String srcOnExit = getInvokeStaticSrc(SQLDriverManagerHook.class, "onConnectionExit", "");
        insertBefore(ctClass, "connect", null, srcBefore);
        insertAfter(ctClass, "connect", null, srcAfter, false);
        insertAfter(ctClass, "connect", null, srcOnExit, true);
    }

    /**
     * 进入数据库连接函数调用的检测入口
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnectionOnEnter(String url, Properties properties) {
        HookHandler.preShieldHook();
    }

    /**
     * 检测sql连接规范
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnectionOnExit(Connection connection, String url, Properties properties) {
        //连接成功才检测
        if (connection == null) {
            return;
        }
        if (!Config.getConfig().getEnforcePolicy()) {
            Long lastAlarmTime = SqlConnectionChecker.alarmTimeCache.get(url);
            if (lastAlarmTime == null || (System.currentTimeMillis() - lastAlarmTime) > TimeUtils.DAY_MILLISECOND) {
                checkSqlConnection(url, properties);
            }
        } else {
            checkSqlConnection(url, properties);
        }
    }

    /**
     * 关闭对 hook 点检测的屏蔽
     */
    public static void onConnectionExit() {
        HookHandler.postShieldHook();
    }

    /**
     * 退出数据库连接函数调用的检测入口
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnection(String url, Properties properties) {
        if (Config.getConfig().getCloudSwitch() && Config.getConfig().getHookWhiteAll()) {
            return;
        }
        HashMap<String, Object> params = new HashMap<String, Object>(4);
        params.put("url", url);
        params.put("properties", properties);
        HookHandler.doRealCheckWithoutRequest(CheckParameter.Type.POLICY_SQL_CONNECTION, params);
    }
}
