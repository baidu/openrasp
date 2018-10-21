package com.baidu.openrasp.cloud;

import com.baidu.openrasp.messaging.SyslogTcpAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;

/**
 * @description: 根据配置动态开启syslog
 * @author: anyang
 * @create: 2018/10/12 11:35
 */
public class DynamicConfigSyslogAppender {
    private static final String LOGGER_NAME = "com.baidu.openrasp.plugin.checker.alarm";
    private static final String APPENDER_NAME = "SYSLOGTCP";

    public static void createSyslogAppender(String address, int port) {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(APPENDER_NAME) != null){
            return;
        }
        SyslogTcpAppender appender = new SyslogTcpAppender(address,port,8);
        appender.setName(APPENDER_NAME);
        appender.setThreshold(Level.INFO);
        appender.setFacilityPrinting(true);
        appender.setReconnectionDelay(60000);
        logger.addAppender(appender);
    }

    public static void removeSyslogAppender() {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(APPENDER_NAME) != null) {
            logger.removeAppender(APPENDER_NAME);
        }
    }
}
