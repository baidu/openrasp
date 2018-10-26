package com.baidu.openrasp.cloud.syslog;

import com.baidu.openrasp.cloud.httpappender.HttpAppender;
import com.baidu.openrasp.messaging.BurstFilter;
import com.baidu.openrasp.messaging.SyslogTcpAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;

/**
 * @description: 根据配置动态开启syslog
 * @author: anyang
 * @create: 2018/10/12 11:35
 */
public class DynamicConfigAppender {
    public static final String LOGGER_NAME = "com.baidu.openrasp.plugin.checker.alarm";
    public static final String POLICY_LOGGER_NAME = "com.baidu.openrasp.plugin.checker.policy_alarm";
    private static final String SYSLOG_APPENDER_NAME = "SYSLOGTCP";
    private static final String HTTP_APPENDER_NAME = "HTTPAPPENDER";

    public static void createSyslogAppender(String address, int port) {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null){
            return;
        }
        RaspCustomLayout layout = new RaspCustomLayout();
        layout.setConversionPattern("%e: %m%n");
        SyslogTcpAppender appender = new SyslogTcpAppender(address,port,8,layout);
        appender.setName(SYSLOG_APPENDER_NAME);
        appender.setThreshold(Level.INFO);
        appender.setFacilityPrinting(true);
        appender.setReconnectionDelay(60000);
        logger.addAppender(appender);
    }

    public static void removeSyslogAppender() {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null) {
            logger.removeAppender(SYSLOG_APPENDER_NAME);
        }
    }

    public static void updateSyslogTag(){
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null) {
            RaspCustomLayout layout = new RaspCustomLayout();
            layout.setConversionPattern("%e -%m%n");
            logger.getAppender(SYSLOG_APPENDER_NAME).setLayout(layout);
        }
    }

    public static void createHttpAppender(String loggerName){
        Logger logger = Logger.getLogger(loggerName);
        HttpAppender appender = new HttpAppender();
        appender.setName(HTTP_APPENDER_NAME);
        BurstFilter filter = new BurstFilter();
        filter.setMaxBurst(100);
        filter.setRefillAmount(100);
        filter.setRefillInterval(1000);
        appender.addFilter(filter);
        logger.addAppender(appender);
    }
}
