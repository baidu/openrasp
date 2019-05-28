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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.info.SecurityPolicyInfo;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.util.*;

public class SqlConnectionChecker extends PolicyChecker {
    public SqlConnectionChecker() {
        super();
    }

    public SqlConnectionChecker(boolean canBlock) {
        super(canBlock);
    }

    private static final String SQL_TYPE_SQLSERVRE = "sqlserver";
    private static final String SQL_TYPE_ORACLE = "oracle";
    private static final String SQL_TYPE_POSTGRESQL = "postgresql";
    private static final String SQL_TYPE_MYSQL = "mysql";
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    private static final String CONNECTION_USER_KEY = "user";
    private static final int ALARM_TIME_CACHE_MAX_SIZE = 5000;
    private static final String DEFAULT_MYSQL_PORT = "3306";
    private static final String DEFAULT_ORACLE_PORT = "1521";
    private static final String DEFAULT_POSTGRESQL_PORT = "5432";
    private static final String DEFAULT_SQLSERVRE_PORT = "1433";
    private static final String[] WEAK_WORDS = new String[]{"root", "123", "123456", "a123456", "123456a", "111111",
            "123123", "admin", "user", "mysql", ""};
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

    private boolean checkPassword(String password) {
        List<String> checkList = Arrays.asList(WEAK_WORDS);
        return !checkList.contains(password);
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {

        String url = (String) checkParameter.getParam("url");
        Properties properties = (Properties) checkParameter.getParam("properties");
        LinkedList<EventInfo> infos = null;
        String sqlType = null;
        String user = null;
        String urlWithoutParams = null;
        String password = null;
        String socket = null;
        String host = null;
        String port = null;
        try {
            Map<String, String> map = parseConnectionString(url);
            if (map != null) {
                sqlType = map.get("type");
                user = map.get("user");
                if (user == null) {
                    user = properties.getProperty(CONNECTION_USER_KEY);
                }
                password = map.get("password");
                if (password == null) {
                    password = properties.getProperty("password");
                }
                urlWithoutParams = map.get("urlWithoutParams");
                socket = map.get("socket");
                host = map.get("host");
                port = map.get("port");
            }
        } catch (Exception e) {
            String message = "check sql connection fail";
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }

        boolean isSafe = checkUser(user, sqlType);
        if (!isSafe) {
            if (alarmTimeCache.size() > ALARM_TIME_CACHE_MAX_SIZE) {
                alarmTimeCache.clear();
            }
            alarmTimeCache.put(url, System.currentTimeMillis());
            String unsafeMessage = "Database security baseline - Connecting to a " + sqlType +
                    " instance with high privileged account " + user +
                    ", connectionString is " + urlWithoutParams;
            infos = new LinkedList<EventInfo>();
            HashMap<String, Object> params = new HashMap<String, Object>(6);
            params.put("server", sqlType != null ? sqlType : "");
            params.put("connectionString", urlWithoutParams != null ? urlWithoutParams : "");
            params.put("username", user != null ? user : "");
            params.put("hostname", host != null ? host : "");
            params.put("port", port != null ? port : "");
            params.put("socket", socket != null ? socket : "");
            infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.SQL_CONNECTION, unsafeMessage, true, params));
        }
        isSafe = checkPassword(password);
        if (!isSafe) {
            if (alarmTimeCache.size() > ALARM_TIME_CACHE_MAX_SIZE) {
                alarmTimeCache.clear();
            }
            alarmTimeCache.put(url, System.currentTimeMillis());
            String unsafeMessage = "Database security baseline - detected weak password for \"" + user + "\" account, password is \"" + password + "\"";
            if (infos == null) {
                infos = new LinkedList<EventInfo>();
            }
            HashMap<String, Object> params = new HashMap<String, Object>(4);
            params.put("server", sqlType != null ? sqlType : "");
            params.put("connectionString", urlWithoutParams != null ? urlWithoutParams : "");
            params.put("username", user != null ? user : "");
            params.put("password", password);
            infos.add(new SecurityPolicyInfo(SecurityPolicyInfo.Type.MANAGER_PASSWORD, unsafeMessage, true, params));
        }
        return infos;
    }

    private Map<String, String> parseConnectionString(String jdbcUrl) {
        if (StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:mysql://") ||
                StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:postgresql:")) {
            return parseUrl(jdbcUrl);
        } else if (StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:sqlserver://")) {
            return parseSqlServerUrl(jdbcUrl);
        } else if (StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:oracle:thin")) {
            return parseOracleUrl(jdbcUrl);
        }
        return null;
    }

    private Map<String, String> parseSqlServerUrl(String jdbcUrl) {
        HashMap<String, String> map = new HashMap<String, String>();
        if (jdbcUrl == null || !StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:sqlserver://")) {
            return map;
        }
        int pos1 = jdbcUrl.indexOf(':', 5);
        if (pos1 == -1) {
            return map;
        }
        String type = jdbcUrl.substring(5, pos1);
        map.put("type", type);
        int pos2 = jdbcUrl.indexOf('?', pos1);
        if (pos2 == -1) {
            pos2 = jdbcUrl.length();
        } else {
            String paramString = jdbcUrl.substring(pos2 + 1);
            parseParam(paramString, map);
        }
        int pos3 = jdbcUrl.indexOf(";", pos1);
        String connUri = jdbcUrl.substring(pos1 + 1, pos3);
        int pos4 = connUri.indexOf(":");
        if (pos4 != -1) {
            String host = connUri.substring(2, pos4);
            String port = connUri.substring(pos4 + 1);
            map.put("host", host);
            map.put("port", port);
        } else {
            String host = connUri.substring(2);
            map.put("host", host);
            map.put("port", DEFAULT_SQLSERVRE_PORT);
        }
        String urlWithoutParams = jdbcUrl.substring(0, pos2);
        map.put("urlWithoutParams", urlWithoutParams);
        return map;
    }

    private Map<String, String> parseUrl(String jdbcUrl) {
        HashMap<String, String> map = new HashMap<String, String>();
        if (jdbcUrl == null || !StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:mysql://") &&
                !StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:postgresql:")) {
            return map;
        }
        int pos1 = jdbcUrl.indexOf(':', 5);
        if (pos1 == -1) {
            return map;
        }
        String type = jdbcUrl.substring(5, pos1);
        map.put("type", type);
        int pos2 = jdbcUrl.indexOf('?', pos1);
        if (pos2 == -1) {
            pos2 = jdbcUrl.length();
        } else {
            String paramString = jdbcUrl.substring(pos2 + 1);
            parseParam(paramString, map);
        }
        String connUri = jdbcUrl.substring(pos1 + 1, pos2);
        int pos3 = connUri.indexOf(":");
        int pos4 = connUri.indexOf('/', 2);
        if (pos3 != -1) {
            String host = connUri.substring(2, pos3);
            map.put("host", host);
            if (pos4 != -1) {
                String port = connUri.substring(pos3 + 1, pos4);
                map.put("port", port);
            }
        } else {
            String host = connUri.substring(2, pos4);
            map.put("host", host);
            if (StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:mysql://")) {
                map.put("port", DEFAULT_MYSQL_PORT);
            } else {
                map.put("port", DEFAULT_POSTGRESQL_PORT);
            }
        }
        String urlWithoutParams = jdbcUrl.substring(0, pos2);
        map.put("urlWithoutParams", urlWithoutParams);
        return map;
    }


    private Map<String, String> parseOracleUrl(String jdbcUrl) {
        HashMap<String, String> map = new HashMap<String, String>();
        if (jdbcUrl == null || !StringUtils.startsWithIgnoreCase(jdbcUrl, "jdbc:oracle:thin")) {
            return map;
        }
        int pos1 = jdbcUrl.indexOf(':', 5);
        if (pos1 == -1) {
            return map;
        }
        String type = jdbcUrl.substring(5, pos1);
        map.put("type", type);
        int pos2 = jdbcUrl.indexOf('?', pos1);
        if (pos2 == -1) {
            pos2 = jdbcUrl.length();
        } else {
            String paramString = jdbcUrl.substring(pos2 + 1);
            parseParam(paramString, map);
        }
        int pos3 = jdbcUrl.indexOf("@");
        if (pos3 != -1) {
            String connUri = jdbcUrl.substring(pos3, pos2);
            int pos4 = connUri.indexOf(":");
            if (pos4 != -1) {
                String host = connUri.substring(1, pos4);
                map.put("host", host);
                int pos5 = connUri.lastIndexOf(":");
                if (pos5 != pos4) {
                    String port = connUri.substring(pos4 + 1, pos5);
                    map.put("port", port);
                } else {
                    map.put("port", DEFAULT_ORACLE_PORT);
                }
            }
        }
        String urlWithoutParams = jdbcUrl.substring(0, pos2);
        map.put("urlWithoutParams", urlWithoutParams);
        return map;
    }

    private void parseParam(String paramString, Map<String, String> map) {
        StringTokenizer queryParams = new StringTokenizer(paramString, "&");
        while (queryParams.hasMoreTokens()) {
            String parameterValuePair = queryParams.nextToken();
            int indexOfEquals = parameterValuePair.indexOf("=");
            if (indexOfEquals > 0) {
                String key = parameterValuePair.substring(0, indexOfEquals);
                String value = parameterValuePair.substring(indexOfEquals + 1);
                map.put(key, value);
            }
        }
    }

}
