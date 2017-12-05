/**
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.fuxi.javaagent.hook;

import com.fuxi.javaagent.HookHandler;
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
                            new Method("checkSqlConnection", "(Ljava/lang/String;Ljava/util/Properties;)V"));
                    invokeStatic(Type.getType(HookHandler.class),
                            new Method("preShieldHook", "()V"));
                }

                @Override
                protected void onMethodExit(int i) {
                    invokeStatic(Type.getType(HookHandler.class),
                            new Method("postShieldHook", "()V"));
                }
            };
        }
        return mv;
    }

    /**
     * 检测sql连接规范
     *
     * @param url        连接url
     * @param properties 连接属性
     */
    public static void checkSqlConnection(String url, Properties properties) {
        Long lastAlarmTime = SqlConnectionChecker.alarmTimeCache.get(url);
        if (lastAlarmTime == null || (System.currentTimeMillis() - lastAlarmTime) > TimeUtils.DAY_MILLISECOND) {
            HashMap<String, Object> params = new HashMap<String, Object>(4);
            params.put("url", url);
            params.put("properties", properties);
            HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_SQL_CONNECTION, params);
        }
    }
}
