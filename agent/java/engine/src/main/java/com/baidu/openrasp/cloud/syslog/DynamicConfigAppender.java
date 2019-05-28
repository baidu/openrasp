package com.baidu.openrasp.cloud.syslog;

import com.baidu.openrasp.cloud.httpappender.HttpAppender;
import com.baidu.openrasp.cloud.model.AppenderMappedLogger;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.BurstFilter;
import com.baidu.openrasp.messaging.OpenraspDailyRollingFileAppender;
import com.baidu.openrasp.messaging.SyslogTcpAppender;
import com.baidu.openrasp.tool.FileUtil;
import org.apache.log4j.*;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.helpers.OnlyOnceErrorHandler;

import java.io.File;

/**
 * @description: 根据配置动态开启syslog
 * @author: anyang
 * @create: 2018/10/12 11:35
 */
public class DynamicConfigAppender {
    public static final String LOGGER_NAME = "com.baidu.openrasp.plugin.checker.alarm";
    private static final String SYSLOG_APPENDER_NAME = "SYSLOGTCP";

    public static void createSyslogAppender(String address, int port) {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        RaspCustomLayout layout = new RaspCustomLayout();
        layout.setConversionPattern("%e: %m%n");
        int syslogFacility = Config.getConfig().getSyslogFacility();
        SyslogTcpAppender appender = new SyslogTcpAppender(address, port, syslogFacility, layout);
        appender.setName(SYSLOG_APPENDER_NAME);
        appender.setThreshold(Level.INFO);
        appender.setFacilityPrinting(true);
        appender.setReconnectionDelay(Config.getConfig().getSyslogReconnectInterval());
        logger.addAppender(appender);
    }

    public static void removeSyslogAppender() {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null) {
            logger.removeAppender(SYSLOG_APPENDER_NAME);
        }
    }

    public static void updateSyslogTag() {
        Logger logger = Logger.getLogger(LOGGER_NAME);
        if (logger.getAppender(SYSLOG_APPENDER_NAME) != null) {
            RaspCustomLayout layout = new RaspCustomLayout();
            layout.setConversionPattern("%e -%m%n");
            logger.getAppender(SYSLOG_APPENDER_NAME).setLayout(layout);
        }
    }

    public static void createHttpAppender(String loggerName, String appenderName) {
        Logger logger = Logger.getLogger(loggerName);
        if (logger.getAppender(appenderName) != null) {
            logger.removeAppender(appenderName);
        }
        HttpAppender appender = new HttpAppender();
        appender.setName(appenderName);
        appender.addFilter(createBurstFilter());
        logger.addAppender(appender);
    }

    public static void createRootHttpAppender() {
        Logger logger = Logger.getRootLogger();
        HttpAppender appender = new HttpAppender();
        appender.setName(AppenderMappedLogger.HTTP_ROOT.getAppender());
        appender.addFilter(createBurstFilter());
        logger.addAppender(appender);
    }

    /**
     * 初始化log4j的logger，并添加fileAppender
     */
    public static void initLog4jLogger() throws Exception {
        for (AppenderMappedLogger type : AppenderMappedLogger.values()) {
            if (type.ordinal() <= 3) {
                if ("root".equals(type.getLogger())) {
                    BasicConfigurator.configure(createFileAppender(type.getAppender(), type.getTargetPath()));
                    Logger.getRootLogger().setLevel(Level.INFO);
                } else {
                    Logger logger = Logger.getLogger(type.getLogger());
                    logger.setLevel(Level.INFO);
                    logger.setAdditivity(false);
                    logger.addAppender(createFileAppender(type.getAppender(), type.getTargetPath()));
                }
            }
        }

        setLogMaxBackup();
        //初始化时是否开启log4j的debug的功能
        enableDebug();
    }

    /**
     * 创建fileAppender
     */
    public static OpenraspDailyRollingFileAppender createFileAppender(String appender, String targetPath) throws Exception {
        OpenraspDailyRollingFileAppender fileAppender = new OpenraspDailyRollingFileAppender();
        fileAppender.setName(appender);
        fileAppender.setErrorHandler(new OnlyOnceErrorHandler());
        String raspBaseDir = FileUtil.getBaseDir();
        fileAppender.setFile(raspBaseDir + targetPath);
        fileAppender.setAppend(true);
        fileAppender.setDatePattern("'.'yyyy-MM-dd");
        fileAppender.setEncoding("UTF-8");
        PatternLayout layout = new PatternLayout();
        if ("PLUGIN".equals(appender) || "RASP".equals(appender)) {
            layout.setConversionPattern("%d %-5p [%t][%c] %m%n");
        } else {
            layout.setConversionPattern("%m%n");
        }
        fileAppender.setLayout(layout);
        fileAppender.activateOptions();
        return fileAppender;
    }

    /**
     * 为fileAppender添加限速filter
     */
    public static void fileAppenderAddBurstFilter() {
        for (AppenderMappedLogger type : AppenderMappedLogger.values()) {
            if (type.ordinal() <= 3) {
                if ("root".equals(type.getLogger())) {
                    Appender appender = Logger.getRootLogger().getAppender(type.getAppender());
                    if (appender instanceof FileAppender) {
                        appender.clearFilters();
                        appender.addFilter(createBurstFilter());
                    }
                } else {
                    Logger logger = Logger.getLogger(type.getLogger());
                    Appender appender = logger.getAppender(type.getAppender());
                    if (appender instanceof FileAppender) {
                        appender.clearFilters();
                        appender.addFilter(createBurstFilter());
                    }
                }
            }
        }
    }

    /**
     * 为httpAppender添加限速filter
     */
    public static void httpAppenderAddBurstFilter() {
        Logger.getRootLogger().getAppender(AppenderMappedLogger.HTTP_ROOT.getAppender()).clearFilters();
        Logger.getRootLogger().getAppender(AppenderMappedLogger.HTTP_ROOT.getAppender()).addFilter(createBurstFilter());
        Logger.getLogger(AppenderMappedLogger.HTTP_ALARM.getLogger()).getAppender(AppenderMappedLogger.HTTP_ALARM.getAppender()).clearFilters();
        Logger.getLogger(AppenderMappedLogger.HTTP_ALARM.getLogger()).getAppender(AppenderMappedLogger.HTTP_ALARM.getAppender()).addFilter(createBurstFilter());
        Logger.getLogger(AppenderMappedLogger.HTTP_POLICY_ALARM.getLogger()).getAppender(AppenderMappedLogger.HTTP_POLICY_ALARM.getAppender()).clearFilters();
        Logger.getLogger(AppenderMappedLogger.HTTP_POLICY_ALARM.getLogger()).getAppender(AppenderMappedLogger.HTTP_POLICY_ALARM.getAppender()).addFilter(createBurstFilter());
    }

    /**
     * 创建日志限速filter
     */
    public static BurstFilter createBurstFilter() {
        BurstFilter filter = new BurstFilter();
        int logMaxBurst = Config.getConfig().getLogMaxBurst();
        filter.setMaxBurst(logMaxBurst);
        filter.setRefillAmount(logMaxBurst);
        filter.setRefillInterval(60);
        return filter;
    }

    /**
     * log4j debug开关
     */
    public static void enableDebug() {
        if (Config.getConfig().isDebugEnabled()) {
            LogLog.setInternalDebugging(true);
            System.out.println("[OpenRASP] Log4j debug mode enabled");
        } else {
            LogLog.setInternalDebugging(false);
            System.out.println("[OpenRASP] Log4j debug mode disabled");
        }
    }

    /**
     * 为fileAppender设置最大日志备份天数
     */
    public static void setLogMaxBackup() {
        int logMaxBackup = Config.getConfig().getLogMaxBackUp();
        for (AppenderMappedLogger type : AppenderMappedLogger.values()) {
            if (type.ordinal() <= 3) {
                if ("root".equals(type.getLogger())) {
                    Appender appender = Logger.getRootLogger().getAppender(type.getAppender());
                    if (appender instanceof FileAppender) {
                        OpenraspDailyRollingFileAppender fileAppender = (OpenraspDailyRollingFileAppender) appender;
                        fileAppender.setMaxBackupIndex(logMaxBackup);
                        fileAppenderRollFiles(fileAppender);
                    }
                } else {
                    Logger logger = Logger.getLogger(type.getLogger());
                    Appender appender = logger.getAppender(type.getAppender());
                    if (appender instanceof FileAppender) {
                        OpenraspDailyRollingFileAppender fileAppender = (OpenraspDailyRollingFileAppender) appender;
                        fileAppender.setMaxBackupIndex(logMaxBackup);
                        fileAppenderRollFiles(fileAppender);
                    }
                }
            }
        }
    }

    //手动触发日志文件rotate
    private static void fileAppenderRollFiles(OpenraspDailyRollingFileAppender fileAppender) {
        try {
            String fileName = fileAppender.getFile();
            fileAppender.rollFiles(new File(fileName));
        } catch (Exception e) {
            LogLog.warn("the appender " + fileAppender.getName() + " roll failed");
        }
    }
}
