package com.fuxi.javaagent.tool.security;

import com.fuxi.javaagent.HookHandler;
import com.fuxi.javaagent.plugin.PluginManager;
import com.fuxi.javaagent.plugin.event.SecurityPolicyInfo;
import com.fuxi.javaagent.tool.OSUtil;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.util.Properties;
import java.util.StringTokenizer;

public class SqlConnectionChecker {

    public static long lastCheckTimeStamp = 0;
    private static final Logger LOGGER = Logger.getLogger(HookHandler.class.getName());
    private static final String CONNECTION_USER_KEY = "user";

    private String unsafeMessage;
    private String user;
    private String sqlType;

    public SqlConnectionChecker(String url, Properties properties) {
        unsafeMessage = "";
        parseConnectionParams(url, properties);
    }

    public boolean check() {
        lastCheckTimeStamp = System.currentTimeMillis();
        boolean result = true;
        if (!StringUtils.isEmpty(user)) {
            if (OSUtil.isWindows()) {
                if (user.equals("sa")) {
                    result = false;
                }
            } else if (OSUtil.isLinux()) {
                if (user.equals("root")) {
                    result = false;
                }
            }
        }

        if (!result) {
            unsafeMessage += ("使用管理员账号" + user + "登录" + sqlType + "数据库");
            PluginManager.ALARM_LOGGER.info(new SecurityPolicyInfo(unsafeMessage));
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
    }

    public String getUnsafeMessage() {
        return unsafeMessage;
    }

}
