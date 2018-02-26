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

package com.fuxi.javaagent.hook.sql;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.config.Config;
import com.fuxi.javaagent.hook.AbstractClassHook;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.checker.policy.SqlConnectionChecker;
import com.fuxi.javaagent.tool.TimeUtils;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.AdviceAdapter;
import org.objectweb.asm.commons.Method;

import java.util.HashMap;
import java.util.Properties;

/**
 * Created by lxk on 7/6/17.
 * All rights reserved
 */
public class SQLDriverManagerHook extends AbstractClassHook {
    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#getType()
     */
    @Override
    public String getType() {
        return "sql";
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#isClassMatched(String)
     */
    @Override
    public boolean isClassMatched(String className) {
        return "java/sql/DriverManager".equals(className);
    }

    /**
     * (none-javadoc)
     *
     * @see com.fuxi.javaagent.hook.AbstractClassHook#hookMethod(int, String, String, String, String[], MethodVisitor)
     */
    @Override
    public MethodVisitor hookMethod(int access, String name, String desc,
                                    String signature, String[] exceptions, MethodVisitor mv) {
        if (name.equals("getConnection") && desc.startsWith("(Ljava/lang/String;Ljava/util/Properties;"
                + "Ljava/lang/Class")) {
            return new AdviceAdapter(Opcodes.ASM5, mv, access, name, desc) {
                @Override
                protected void onMethodEnter() {
                    loadArg(0);
                    loadArg(1);
                    invokeStatic(Type.getType(SQLDriverManagerHook.class),
                            new Method("checkSqlConnectionOnEnter", "(Ljava/lang/String;Ljava/util/Properties;)V"));
                    invokeStatic(Type.getType(HookHandler.class),
                            new Method("preShieldHook", "()V"));
                }

                @Override
                protected void onMethodExit(int i) {
                    if (Opcodes.ATHROW != i) {
                        loadArg(0);
                        loadArg(1);
                        invokeStatic(Type.getType(SQLDriverManagerHook.class),
                                new Method("checkSqlConnectionOnExit", "(Ljava/lang/String;Ljava/util/Properties;)V"));
                    }
                    invokeStatic(Type.getType(HookHandler.class),
                            new Method("postShieldHook", "()V"));
                }
            };
        }
        return mv;
    }

    /**
     * 进入数据库连接函数调用的检测入口
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnectionOnEnter(String url, Properties properties) {
        if (Config.getConfig().getEnforcePolicy()) {
            checkSqlConnection(url, properties);
        }
    }

    /**
     * 检测sql连接规范
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnectionOnExit(String url, Properties properties) {
        if (!Config.getConfig().getEnforcePolicy()) {
            Long lastAlarmTime = SqlConnectionChecker.alarmTimeCache.get(url);
            if (lastAlarmTime == null || (System.currentTimeMillis() - lastAlarmTime) > TimeUtils.DAY_MILLISECOND) {
                checkSqlConnection(url, properties);
            }
        }
    }

    /**
     * 退出数据库连接函数调用的检测入口
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnection(String url, Properties properties) {
        HashMap<String, Object> params = new HashMap<String, Object>(4);
        params.put("url", url);
        params.put("properties", properties);
        HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_SQL_CONNECTION, params);

    }
}
