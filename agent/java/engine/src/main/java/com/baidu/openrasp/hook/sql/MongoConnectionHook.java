/*
 * Copyright 2017-2021 Baidu Inc.
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
import com.baidu.openrasp.plugin.checker.policy.MongoConnectionChecker;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.TimeUtils;
import com.baidu.openrasp.tool.annotation.HookAnnotation;
import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtClass;
import javassist.NotFoundException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * @description: mongo 连接检测
 * @author: anyang
 * @create: 2019/05/14 15:05
 */
@HookAnnotation
public class MongoConnectionHook extends AbstractClassHook {
    private static final int DEFAULT_MONGO_PORT = 27017;

    @Override
    public boolean isClassMatched(String className) {
        return "com/mongodb/Mongo".equals(className);
    }

    @Override
    public String getType() {
        return "mongodb";
    }

    @Override
    protected void hookMethod(CtClass ctClass) throws IOException, CannotCompileException, NotFoundException {
        String desc1 = "(Lcom/mongodb/ServerAddress;Ljava/util/List;Lcom/mongodb/MongoClientOptions;)V";
        String desc2 = "(Ljava/util/List;Ljava/util/List;Lcom/mongodb/MongoClientOptions;)V";
        String desc3 = "(Lcom/mongodb/MongoClientURI;)V";

        String src1 = getInvokeStaticSrc(MongoConnectionHook.class, "checkSqlConnection",
                "$1,$2", Object.class, List.class);
        String src2 = getInvokeStaticSrc(MongoConnectionHook.class, "checkSqlConnection",
                "$1,$2", List.class, List.class);
        String src3 = getInvokeStaticSrc(MongoConnectionHook.class, "checkSqlConnection",
                "$1", Object.class);
        CtBehavior behavior1 = ctClass.getConstructor(desc1);
        CtBehavior behavior2 = ctClass.getConstructor(desc2);
        CtBehavior behavior3 = ctClass.getConstructor(desc3);
        behavior1.insertAfter(src1, false);
        behavior2.insertAfter(src2, false);
        behavior3.insertAfter(src3, false);
    }

    public static void checkSqlConnection(Object server, List<Object> mongoCredentials) {
        String username = null;
        char[] password = null;
        String database = null;
        if (mongoCredentials != null && !mongoCredentials.isEmpty()) {
            Object mongoCredential = mongoCredentials.get(0);
            username = Reflection.invokeStringMethod(mongoCredential, "getUserName", new Class[]{});
            password = (char[]) Reflection.invokeMethod(mongoCredential, "getPassword", new Class[]{});
            database = Reflection.invokeStringMethod(mongoCredential, "getSource", new Class[]{});
        }
        List<String> hosts = new ArrayList<String>();
        List<Integer> ports = new ArrayList<Integer>();
        if (server != null) {
            String host = Reflection.invokeStringMethod(server, "getHost", new Class[]{});
            Integer port = (Integer) Reflection.invokeMethod(server, "getPort", new Class[]{});
            if (host != null && port != null) {
                hosts.add(host);
                ports.add(port);
            }
        }
        if (username != null && password != null) {
            database = database == null ? "" : database;
            String url = username + new String(password) + hosts.toString() + ports.toString() + database;
            check(username, new String(password), hosts, ports, "", url);
        }
    }

    public static void checkSqlConnection(List<Object> servers, List<Object> mongoCredentials) {
        String username = null;
        char[] password = null;
        String database = null;
        if (mongoCredentials != null && !mongoCredentials.isEmpty()) {
            Object mongoCredential = mongoCredentials.get(0);
            database = Reflection.invokeStringMethod(mongoCredential, "getSource", new Class[]{});
            username = Reflection.invokeStringMethod(mongoCredential, "getUserName", new Class[]{});
            password = (char[]) Reflection.invokeMethod(mongoCredential, "getPassword", new Class[]{});
        }
        List<String> hosts = new ArrayList<String>();
        List<Integer> ports = new ArrayList<Integer>();
        if (servers != null && !servers.isEmpty()) {
            for (Object server : servers) {
                String host = Reflection.invokeStringMethod(server, "getHost", new Class[]{});
                Integer port = (Integer) Reflection.invokeMethod(server, "getPort", new Class[]{});
                if (host != null && port != null) {
                    hosts.add(host);
                    ports.add(port);
                }
            }
        }
        if (username != null && password != null) {
            database = database == null ? "" : database;
            String url = username + new String(password) + hosts.toString() + ports.toString() + database;
            check(username, new String(password), hosts, ports, "", url);
        }
    }

    public static void checkSqlConnection(Object mongoURI) {
        if (mongoURI != null) {
            String username = Reflection.invokeStringMethod(mongoURI, "getUsername", new Class[]{});
            char[] password = (char[]) Reflection.invokeMethod(mongoURI, "getPassword", new Class[]{});
            String connectionString = Reflection.invokeStringMethod(mongoURI, "getURI", new Class[]{});
            List<String> servers = (List<String>) Reflection.invokeMethod(mongoURI, "getHosts", new Class[]{});
            List<String> hosts = new ArrayList<String>();
            List<Integer> ports = new ArrayList<Integer>();
            if (servers != null && !servers.isEmpty()) {
                for (String server : servers) {
                    String[] temp = server.split(":");
                    hosts.add(temp[0]);
                    if (temp.length > 1) {
                        ports.add(Integer.parseInt(temp[1]));
                    } else {
                        ports.add(DEFAULT_MONGO_PORT);
                    }
                }
            }
            if (username != null && password != null) {
                check(username, new String(password), hosts, ports, connectionString, connectionString);
            }
        }
    }

    public static void check(String username, String password, List<String> hosts, List<Integer> ports, String connectionString, String url) {
        if (Config.getConfig().getCloudSwitch() && Config.getConfig().getHookWhiteAll()) {
            return;
        }
        HashMap<String, Object> params = new HashMap<String, Object>(5);
        params.put("username", username);
        params.put("password", password);
        params.put("hosts", hosts);
        params.put("ports", ports);
        params.put("connectionString", connectionString);
        params.put("url", url);
        Long lastAlarmTime = MongoConnectionChecker.alarmTimeCache.get(url);
        if (lastAlarmTime == null || (System.currentTimeMillis() - lastAlarmTime) > TimeUtils.DAY_MILLISECOND) {
            HookHandler.doCheckWithoutRequest(CheckParameter.Type.POLICY_MONGO_CONNECTION, params);
        }
    }
}
