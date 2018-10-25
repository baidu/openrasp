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
    private static final String LOGGER_NAME = "com.baidu.openrasp.plugin.checker.alarm";
    private static final String POLICY_LOGGER_NAME = "com.baidu.openrasp.plugin.checker.policy_alarm";
    private static final String SYSLOG_APPENDER_NAME = "SYSLOGTCP";
    private static final String HTTP_APPENDER_NAME = "HTTPAPPENDER";

    public static void createSyslogAppender(String address, int port) {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null){
            return;
        }
        SyslogTcpAppender appender = new SyslogTcpAppender(address,port,8);
        appender.setName(SYSLOG_APPENDER_NAME);
        appender.setThreshold(Level.INFO);
        appender.setFacilityPrinting(true);
        appender.setReconnectionDelay(60000);
//        RaspCustomLayout layout = new RaspCustomLayout("%e -%m%n");
//        layout.setConversionPattern("%e -%m%n");
        CustomLayout layout = new CustomLayout();
        appender.setLayout(layout);
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
            logger.getAppender(SYSLOG_APPENDER_NAME).setLayout(new CustomLayout());
        }
    }

    public static void createHttpAppenderForAlarm(){
        Logger alarmLogger = Logger.getLogger(LOGGER_NAME);
        HttpAppender appender = new HttpAppender();
        appender.setName(HTTP_APPENDER_NAME);
        BurstFilter filter = new BurstFilter();
        filter.setMaxBurst(100);
        filter.setRefillAmount(100);
        filter.setRefillInterval(1000);
        appender.addFilter(filter);
        alarmLogger.addAppender(appender);
    }

    public static void createHttpAppenderForPolicyAlarm(){
        Logger policyAlarmLogger = Logger.getLogger(POLICY_LOGGER_NAME);
        BurstFilter filter = new BurstFilter();
        filter.setMaxBurst(100);
        filter.setRefillAmount(100);
        filter.setRefillInterval(1000);
        HttpAppender appender = new HttpAppender();
        appender.setName(HTTP_APPENDER_NAME);
        appender.addFilter(filter);
        policyAlarmLogger.addAppender(appender);
    }
}
