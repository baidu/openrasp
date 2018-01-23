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

package com.baidu.openrasp.plugin.checker.policy;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.util.*;

public class SqlConnectionChecker extends PolicyChecker {

    private static final String SQL_TYPE_SQLSERVRE = "sqlserver";
    private static final String SQL_TYPE_ORACLE = "oracle";
    private static final String SQL_TYPE_POSTGRESQL = "postgresql";
    private static final String SQL_TYPE_MYSQL = "mysql";
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    private static final String CONNECTION_USER_KEY = "user";
    private static final int ALARM_TIME_CACHE_MAX_SIZE = 5000;
    public static HashMap<String, Long> alarmTimeCache = new HashMap<String, Long>();

    private boolean checkUser(String user, String sqlType) {
        boolean isSafe = true;
        if (!StringUtils.isEmpty(user)) {

            LinkedList<String> adminUsers = new LinkedList<String>();
            if (SQL_TYPE_MYSQL.equals(sqlType)) {
                adminUsers.add("root");
            } else if (SQL_TYPE_ORACLE.equals(sqlType)) {
                adminUsers.add("sys");
                adminUsers.add("system");
                adminUsers.add("sysman");
                adminUsers.add("dbsnmp");
            } else if (SQL_TYPE_SQLSERVRE.equals(sqlType)) {
                adminUsers.add("sa");
            } else if (SQL_TYPE_POSTGRESQL.equals(sqlType)) {
                adminUsers.add("postgres");
            }
            if (adminUsers.contains(user)) {
                isSafe = false;
            }
        }
        return isSafe;
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {

        String url = (String) checkParameter.getParam("url");
        Properties properties = (Properties) checkParameter.getParam("properties");
        LinkedList<EventInfo> infos = null;
        String sqlType = null;
        String user = null;
        try {
            if (!StringUtils.isEmpty(url) && url.startsWith("jdbc:")) {
                int indexOfPath = url.indexOf(':', 5);
                if (indexOfPath != -1) {
                    sqlType = url.substring(5, indexOfPath);
                }
                if (sqlType != null && sqlType.length() > 1) {
                    if (properties != null) {
                        user = properties.getProperty(CONNECTION_USER_KEY);
                    }
                    if (StringUtils.isEmpty(user)) {
                        int index = url.indexOf("?", indexOfPath);
                        if (index != -1) {
                            String paramString = url.substring(index + 1, url.length());
                            StringTokenizer queryParams = new StringTokenizer(paramString, "&");
                            while (queryParams.hasMoreTokens()) {
                                String parameterValuePair = queryParams.nextToken();
                                int indexOfEquals = parameterValuePair.indexOf("=");
                                if (indexOfEquals > 0) {
                                    if (parameterValuePair.substring(0, indexOfEquals).equals(CONNECTION_USER_KEY)) {
                                        user = parameterValuePair.substring(indexOfEquals + 1, parameterValuePair.length());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            LOGGER.warn("check sql connection fail cause by:" + e.getMessage());
        }

        boolean isSafe = checkUser(user, sqlType);
        if (!isSafe) {
            if (alarmTimeCache.size() > ALARM_TIME_CACHE_MAX_SIZE) {
                alarmTimeCache.clear();
            }
            alarmTimeCache.put(url, System.currentTimeMillis());
            String unsafeMessage = "使用管理员账号" + user + "登录" + sqlType + "数据库:" + url;
            infos = new LinkedList<EventInfo>();
            infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SQL_CONNECTION, unsafeMessage, true));
        }

        return infos;
    }

}
