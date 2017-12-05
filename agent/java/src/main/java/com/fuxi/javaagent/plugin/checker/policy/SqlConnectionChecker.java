/*
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

package com.fuxi.javaagent.plugin.checker.policy;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.checker.CheckParameter;
import com.fuxi.javaagent.plugin.info.EventInfo;
import com.fuxi.javaagent.plugin.info.SecurityPolicyInfo;
import com.fuxi.javaagent.tool.TimeUtils;
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
        Long lastAlarmTime = SqlConnectionChecker.alarmTimeCache.get(url);
        LinkedList<EventInfo> infos = null;
        if (lastAlarmTime == null || (System.currentTimeMillis() - lastAlarmTime) > TimeUtils.DAY_MILLISECOND) {
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
                alarmTimeCache.put(url, System.currentTimeMillis());
                String unsafeMessage = "使用管理员账号" + user + "登录" + sqlType + "数据库:" + url;
                infos = new LinkedList<EventInfo>();
                infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SQL_CONNECTION, unsafeMessage, true));
            }
        }
        return infos;
    }

}
