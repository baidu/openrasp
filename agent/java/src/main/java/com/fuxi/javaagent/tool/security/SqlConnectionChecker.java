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

package com.fuxi.javaagent.tool.security;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.event.SecurityPolicyInfo;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.Properties;
import java.util.StringTokenizer;

public class SqlConnectionChecker implements PolicyChecker {

    public static final String SQL_TYPE_SQLSERVRE = "sqlserver";
    public static final String SQL_TYPE_ORACLE = "oracle";
    public static final String SQL_TYPE_POSTGRESQL = "postgresql";
    public static final String SQL_TYPE_MYSQL = "mysql";
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    private static final String CONNECTION_USER_KEY = "user";
    public static HashMap<String, Long> alarmTimeCache = new HashMap<String, Long>();

    private String unsafeMessage;
    private String user;
    private String sqlType;
    private String url;

    public SqlConnectionChecker(String url, Properties properties) {
        unsafeMessage = "";
        this.url = url;
        parseConnectionParams(url, properties);
    }

    @Override
    public boolean check() {
        boolean result = true;
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
                result = false;
            }
        }

        if (!result) {
            alarmTimeCache.put(url, System.currentTimeMillis());
            unsafeMessage += ("使用管理员账号" + user + "登录" + sqlType + "数据库:" + url);
            PolicyChecker.ALARM_LOGGER.info(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SQL_CONNECTION, unsafeMessage));
        }
        return result;
    }

    private void parseConnectionParams(String url, Properties properties) {
        try {

            if (url.startsWith("jdbc:")) {
                int indexOfPath = url.indexOf(':', 5);
                if (indexOfPath != -1) {
                    sqlType = url.substring(5, indexOfPath);
                }
                if (sqlType != null && sqlType.length() > 1) {
                    if (properties != null) {
                        user = properties.getProperty(CONNECTION_USER_KEY);
                    }
                    if (StringUtils.isEmpty(user)) {
                        String paramStartToken = "?";
                        String paramSplitToken = "&";
                        if (sqlType.toLowerCase().equals(SQL_TYPE_SQLSERVRE)) {
                            paramStartToken = ";";
                            paramSplitToken = ";";
                        }
                        int index = url.indexOf(paramStartToken, indexOfPath);
                        if (index != -1) {
                            String paramString = url.substring(index + 1);
                            StringTokenizer queryParams = new StringTokenizer(paramString, paramSplitToken);
                            while (queryParams.hasMoreTokens()) {
                                String parameterValuePair = queryParams.nextToken();
                                int indexOfEquals = parameterValuePair.indexOf("=");
                                if (indexOfEquals > 0) {
                                    if (parameterValuePair.substring(0, indexOfEquals).equals(CONNECTION_USER_KEY)) {
                                        user = parameterValuePair.substring(indexOfEquals + 1);
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
    }

    public String getUnsafeMessage() {
        return unsafeMessage;
    }

}
