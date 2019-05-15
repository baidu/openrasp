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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import org.apache.commons.lang3.StringUtils;

import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

/**
 * @description: mongo 数据库基线检测
 * @author: anyang
 * @create: 2019/05/14 17:53
 */
public class MongoConnectionChecker extends PolicyChecker {
    private static final String[] adminUsers = new String[]{"admin"};
    private static final String[] WEAK_WORDS = new String[]{"root", "123", "123456", "a123456", "123456a", "111111",
            "123123", "admin", "user", "mysql", ""};
    private static final int ALARM_TIME_CACHE_MAX_SIZE = 5000;
    public static HashMap<String, Long> alarmTimeCache = new HashMap<String, Long>();

    private boolean checkUser(String user) {
        boolean isSafe = true;
        if (!StringUtils.isEmpty(user)) {
            if (Arrays.asList(adminUsers).contains(user)) {
                isSafe = false;
            }
        }
        return isSafe;
    }

    private boolean checkPassword(String password) {
        List<String> checkList = Arrays.asList(WEAK_WORDS);
        return !checkList.contains(password);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        String connectionString = (String) checkParameter.getParam("connectionString");
        LinkedList<EventInfo> infos = null;
        String user = (String) checkParameter.getParam("username");
        String password = (String) checkParameter.getParam("password");
        List<String> hosts = (List<String>) checkParameter.getParam("hosts");
        List<String> ports = (List<String>) checkParameter.getParam("ports");
        String url = (String) checkParameter.getParam("url");
        boolean isSafe = checkUser(user);
        if (!isSafe) {
            if (alarmTimeCache.size() > ALARM_TIME_CACHE_MAX_SIZE) {
                alarmTimeCache.clear();
            }
            alarmTimeCache.put(url, System.currentTimeMillis());
            String unsafeMessage = "Database security baseline - Connecting to a mongo" +
                    " instance with high privileged account " + user +
                    ", connectionString is " + connectionString;
            infos = new LinkedList<EventInfo>();
            HashMap<String, Object> params = new HashMap<String, Object>(6);
            params.put("server", "mongo");
            params.put("connectionString", connectionString);
            params.put("username", user);
            params.put("hostname", hosts);
            params.put("port", ports);
            params.put("socket", "");
            infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SQL_CONNECTION, unsafeMessage, true, params));
        }
        isSafe = checkPassword(password);
        if (!isSafe) {
            if (alarmTimeCache.size() > ALARM_TIME_CACHE_MAX_SIZE) {
                alarmTimeCache.clear();
            }
            alarmTimeCache.put(url, System.currentTimeMillis());
            String unsafeMessage = "Database security baseline - the password \"" + password + "\" is detected weak password combination , username is " + user;
            if (infos == null) {
                infos = new LinkedList<EventInfo>();
            }
            HashMap<String, Object> params = new HashMap<String, Object>(4);
            params.put("server", "mongo");
            params.put("connectionString", connectionString);
            params.put("username", user);
            params.put("password", password);
            infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.MANAGER_PASSWORD, unsafeMessage, true, params));
        }
        return infos;
    }
}
