/*
 * Copyright 2017-2020 Baidu Inc.
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

package com.baidu.openrasp.cloud.syslog;

import com.baidu.openrasp.cloud.httpappender.HttpAppender;
import com.baidu.openrasp.cloud.model.AppenderMappedLogger;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.messaging.BurstFilter;
import com.baidu.openrasp.messaging.OpenraspDailyRollingFileAppender;
import com.baidu.openrasp.messaging.SyslogTcpAppender;
import com.baidu.openrasp.tool.FileUtil;
import com.baidu.openrasp.tool.FilterConstructor;
import org.apache.log4j.*;
import org.apache.log4j.helpers.LogLog;
import org.apache.log4j.helpers.OnlyOnceErrorHandler;
import org.apache.log4j.varia.NullAppender;
import org.yaml.snakeyaml.Yaml;

import java.io.File;
import java.io.FileInputStream;
import java.util.Map;

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
    public static void initLog4jLogger() {
        String log4jBaseDir = getLog4jPath(false, null);
        if (log4jBaseDir == null) {
            log4jBaseDir = FileUtil.getBaseDir();
        }
        for (AppenderMappedLogger type : AppenderMappedLogger.values()) {
            if (type.ordinal() <= 3) {
                if ("root".equals(type.getLogger())) {
                    if (log4jBaseDir.isEmpty()) {
                        Logger.getRootLogger().addAppender(createNullAppender(type.getAppender()));
                        Logger.getRootLogger().setLevel(Level.INFO);
                    } else {
                        if (!(new File(log4jBaseDir).exists() && new File(log4jBaseDir).isDirectory())) {
                            log4jBaseDir = FileUtil.getBaseDir();
                        }
                        BasicConfigurator.configure(createFileAppender(type.getAppender(), log4jBaseDir + type.getTargetPath()));
                        Logger.getRootLogger().setLevel(Level.INFO);

                    }
                } else {
                    Logger logger = Logger.getLogger(type.getLogger());
                    if (log4jBaseDir.isEmpty()) {
                        logger.addAppender(createNullAppender(type.getAppender()));
                        logger.setLevel(Level.INFO);
                        logger.setAdditivity(false);
                    } else {
                        if (!(new File(log4jBaseDir).exists() && new File(log4jBaseDir).isDirectory())) {
                            log4jBaseDir = FileUtil.getBaseDir();
                        }
                        logger.addAppender(createFileAppender(type.getAppender(), log4jBaseDir + type.getTargetPath()));
                        logger.setLevel(Level.INFO);
                        logger.setAdditivity(false);
                    }
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
    public static OpenraspDailyRollingFileAppender createFileAppender(String appender, String targetPath) {
        OpenraspDailyRollingFileAppender fileAppender = new OpenraspDailyRollingFileAppender();
        fileAppender.setName(appender);
        fileAppender.setErrorHandler(new OnlyOnceErrorHandler());
        fileAppender.setFile(targetPath);
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
     * 创建NullAppender
     */
    public static NullAppender createNullAppender(String appenderName) {
        NullAppender appender = new NullAppender();
        appender.setName(appenderName);
        return appender;
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

    /**
     * 获取log4j的日志自定义路径
     */
    private static String getLog4jPath(boolean isCloud, String path) {
        if (isCloud) {
            return path;
        } else {
            String configPath = FileUtil.getBaseDir() + File.separator + "conf" + File.separator + "openrasp.yml";
            File configFile = new File(configPath);
            if (configFile.exists()) {
                try {
                    Yaml yaml = new Yaml(new FilterConstructor());
                    Map<String, Object> configMap = yaml.loadAs(new FileInputStream(configPath), Map.class);
                    if (configMap != null) {
                        Boolean cloudEnable = (Boolean) configMap.get("cloud.enable");
                        if (cloudEnable != null && cloudEnable) {
                            return null;
                        } else {
                            return (String) configMap.get("log.path");
                        }
                    }
                } catch (Exception e) {
                    String message = "get log4j custom path failed from openrasp.yml";
                    LogLog.warn(message, e);
                }
            }
            return null;
        }
    }

    /**
     * 更新log4j的日志自定义路径
     */
    public static void updateLog4jPath(boolean isCloud, String path) {
        String log4jBaseDir = getLog4jPath(isCloud, path);
        if (log4jBaseDir == null) {
            return;
        }
        for (AppenderMappedLogger type : AppenderMappedLogger.values()) {
            if (type.ordinal() <= 3) {
                Logger logger;
                if ("root".equals(type.getLogger())) {
                    logger = Logger.getRootLogger();
                } else {
                    logger = Logger.getLogger(type.getLogger());
                }
                if (log4jBaseDir.isEmpty()) {
                    logger.removeAppender(type.getAppender());
                    logger.addAppender(createNullAppender(type.getAppender()));
                } else {
                    if (!(new File(log4jBaseDir).exists() && new File(log4jBaseDir).isDirectory() && new File(log4jBaseDir).canWrite())) {
                        return;
                    }
                    Appender appender = logger.getAppender(type.getAppender());
                    if (appender instanceof FileAppender) {
                        if (!(log4jBaseDir + type.getTargetPath()).equals(((FileAppender) appender).getFile())) {
                            ((FileAppender) appender).setFile(log4jBaseDir + type.getTargetPath());
                        }
                    } else if (appender instanceof NullAppender) {
                        logger.removeAppender(type.getAppender());
                        logger.addAppender(createFileAppender(type.getAppender(), log4jBaseDir + type.getTargetPath()));
                    }
                }
            }
        }
    }
}
