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

package com.baidu.openrasp.config;

import com.baidu.openrasp.exception.ConfigLoadException;
import com.baidu.openrasp.tool.FileUtil;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

import java.io.*;
import java.net.URLDecoder;
import java.util.Properties;


/**
 * Created by tyy on 3/27/17.
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config extends FileScanListener {

    public enum Item {
        PLUGIN_TIMEOUT_MILLIS("plugin.timeout.millis", "100"),
        HOOKS_IGNORE("hooks.ignore", ""),
        BLOCK_URL("block.url", "https://rasp.baidu.com/blocked"),
        READ_FILE_EXTENSION_REGEX("readfile.extension.regex", "^(gz|7z|xz|tar|rar|zip|sql|db)$"),
        INJECT_URL_PREFIX("inject.urlprefix", ""),
        BODY_MAX_BYTES("body.maxbytes", "4096"),
        LOG_MAX_STACK("log.maxstack", "20"),
        REFLECTION_MAX_STACK("plugin.maxstack", "100"),
        SECURITY_ENFORCE_POLICY("security.enforce_policy", "false"),
        OGNL_EXPRESSION_MIN_LENGTH("ognl.expression.minlength", "30"),
        SQL_SLOW_QUERY_MIN_ROWS("sql.slowquery.min_rows", "500"),
        BLOCK_STATUS_CODE("block.status_code", "302"),
        DEBUG("debug_level", "0"),
        ALGORITHM_CONFIG("algorithm.config", "{}", false);


        Item(String key, String defaultValue) {
            this(key, defaultValue, true);
        }

        Item(String key, String defaultValue, boolean isProperties) {
            this.key = key;
            this.defaultValue = defaultValue;
            this.isProperties = isProperties;
        }

        String key;
        String defaultValue;
        boolean isProperties;

        @Override
        public String toString() {
            return key;
        }
    }

    private static final String CONFIG_DIR_NAME = "conf";
    private static final String CONFIG_FILE_NAME = "rasp.properties";
    public static final int REFLECTION_STACK_START_INDEX = 0;
    public static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    public static String baseDirectory;
    private static Integer watchId;

    private String configFileDir;
    private int pluginMaxStack;
    private long pluginTimeout;
    private int bodyMaxBytes;
    private int sqlSlowQueryMinCount;
    private String[] ignoreHooks;
    private boolean enforcePolicy;
    private String[] reflectionMonitorMethod;
    private int logMaxStackSize;
    private String readFileExtensionRegex;
    private String blockUrl;
    private String injectUrlPrefix;
    private int ognlMinLength;
    private int blockStatusCode;
    private int debugLevel;
    private JsonObject algorithmConfig;

    static {
        baseDirectory = FileUtil.getBaseDir();
        CustomResponseHtml.load(baseDirectory);
        try {
            FileScanMonitor.addMonitor(
                    baseDirectory, ConfigHolder.instance);
        } catch (JNotifyException e) {
            throw new ConfigLoadException("add listener on " + baseDirectory + " failed because:" + e.getMessage());
        }
        LOGGER.info("baseDirectory: " + baseDirectory);
    }

    /**
     * 构造函数，初始化全局配置
     */
    private Config() {
        this.configFileDir = baseDirectory + File.separator + CONFIG_DIR_NAME;
        String configFilePath = this.configFileDir + File.separator + CONFIG_FILE_NAME;
        try {
            loadConfigFromFile(new File(configFilePath), true);
            addConfigFileMonitor();
        } catch (FileNotFoundException e) {
            handleException("Could not find rasp.properties, using default settings: " + e.getMessage(), e);
        } catch (JNotifyException e) {
            handleException("add listener on " + configFileDir + " failed because:" + e.getMessage(), e);
        } catch (IOException e) {
            handleException("cannot load properties file: " + e.getMessage(), e);
        }
    }

    private synchronized void loadConfigFromFile(File file, boolean isInit) throws IOException {
        Properties properties = new Properties();
        try {
            if (file.exists()) {
                FileInputStream input = new FileInputStream(file);
                properties.load(input);
                input.close();
            }
        } finally {
            // 出现解析问题使用默认值
            for (Item item : Item.values()) {
                if (item.isProperties) {
                    setConfigFromProperties(item, properties, isInit);
                }
            }
        }
    }

    private void reloadConfig(File file) {
        if (file.getName().equals(CONFIG_FILE_NAME)) {
            try {
                loadConfigFromFile(file, false);
            } catch (IOException e) {
                LOGGER.warn("update rasp.properties failed because: " + e.getMessage());
            }
        }
    }

    private void addConfigFileMonitor() throws JNotifyException {
        if (watchId != null) {
            FileScanMonitor.removeMonitor(watchId);
        }
        watchId = FileScanMonitor.addMonitor(configFileDir, new FileScanListener() {
            @Override
            public void onFileCreate(File file) {
                reloadConfig(file);
            }

            @Override
            public void onFileChange(File file) {
                reloadConfig(file);
            }

            @Override
            public void onFileDelete(File file) {
                reloadConfig(file);
            }
        });
    }

    private void setConfigFromProperties(Item item, Properties properties, boolean isInit) {
        String key = item.key;
        String value = properties.getProperty(item.key, item.defaultValue);
        try {
            setConfig(key, value, isInit);
        } catch (Exception e) {
            // 出现解析问题使用默认值
            value = item.defaultValue;
            setConfig(key, item.defaultValue, false);
            LOGGER.warn("set config " + item.key + " failed, use default value : " + value);
        }
    }

    private void handleException(String message, Exception e) {
        LOGGER.warn(message);
        System.out.println(message);
    }

    private static class ConfigHolder {
        static Config instance = new Config();
    }

    /**
     * 获取配置单例
     *
     * @return Config单例对象
     */
    public static Config getConfig() {
        return ConfigHolder.instance;
    }

    /**
     * 获取当前jar包所在目录
     *
     * @return 当前jar包目录
     */
    public String getBaseDirectory() {
        return baseDirectory;
    }

    /**
     * 获取js脚本所在目录
     *
     * @return js脚本目录
     */
    public String getScriptDirectory() {
        return baseDirectory + "/plugins";
    }

    /**
     * 获取自定义插入 html 页面的 js 脚本
     *
     * @return js脚本内容
     */
    public String getCustomResponseScript() {
        return CustomResponseHtml.getInstance() != null ? CustomResponseHtml.getInstance().getContent() : null;
    }

    @Override
    public void onDirectoryCreate(File file) {
        reloadConfigDir(file);
    }

    @Override
    public void onDirectoryDelete(File file) {
        reloadConfigDir(file);
    }

    @Override
    public void onFileCreate(File file) {
        // ignore
    }

    @Override
    public void onFileChange(File file) {
        // ignore
    }

    @Override
    public void onFileDelete(File file) {
        // ignore
    }

    private void reloadConfigDir(File directory) {
        try {
            if (directory.getName().equals(CustomResponseHtml.CUSTOM_RESPONSE_BASE_DIR)) {
                CustomResponseHtml.load(baseDirectory);
            } else if (directory.getName().equals(CONFIG_DIR_NAME)) {
                reloadConfig(new File(configFileDir + File.separator + CONFIG_FILE_NAME));
            }
        } catch (Exception e) {
            LOGGER.warn("update " + directory.getAbsolutePath() + " failed because: " + e.getMessage());
        }
    }

    //--------------------可以通过插件修改的配置项-----------------------------------

    /**
     * 获取Js引擎执行超时时间
     *
     * @return 超时时间
     */
    public synchronized long getPluginTimeout() {
        return pluginTimeout;
    }

    /**
     * 配置Js引擎执行超时时间
     *
     * @param pluginTimeout 超时时间
     */
    public synchronized void setPluginTimeout(String pluginTimeout) {
        this.pluginTimeout = Long.parseLong(pluginTimeout);
        if (this.pluginTimeout < 0) {
            this.pluginTimeout = 0;
        }
    }

    /**
     * 设置需要插入自定义html的页面path前缀
     *
     * @return 页面path前缀
     */
    public synchronized String getInjectUrlPrefix() {
        return injectUrlPrefix;
    }

    /**
     * 获取需要插入自定义html的页面path前缀
     *
     * @param injectUrlPrefix 页面path前缀
     */
    public synchronized void setInjectUrlPrefix(String injectUrlPrefix) {
        StringBuilder injectPrefix = new StringBuilder(injectUrlPrefix);
        while (injectPrefix.length() > 0 && injectPrefix.charAt(injectPrefix.length() - 1) == '/') {
            injectPrefix.deleteCharAt(injectPrefix.length() - 1);
        }
        this.injectUrlPrefix = injectPrefix.toString();
    }

    /**
     * 保存HTTP请求体时最大保存长度
     *
     * @return 最大长度
     */
    public synchronized int getBodyMaxBytes() {
        return bodyMaxBytes;
    }

    /**
     * 配置保存HTTP请求体时最大保存长度
     *
     * @param bodyMaxBytes
     */
    public synchronized void setBodyMaxBytes(String bodyMaxBytes) {
        this.bodyMaxBytes = Integer.parseInt(bodyMaxBytes);
        if (this.bodyMaxBytes < 0) {
            this.bodyMaxBytes = 0;
        }
    }

    public synchronized int getSqlSlowQueryMinCount() {
        return sqlSlowQueryMinCount;
    }

    public synchronized void setSqlSlowQueryMinCount(String sqlSlowQueryMinCount) {
        this.sqlSlowQueryMinCount = Integer.parseInt(sqlSlowQueryMinCount);
        if (this.sqlSlowQueryMinCount < 0) {
            this.sqlSlowQueryMinCount = 0;
        }
    }

    /**
     * 需要忽略的挂钩点
     *
     * @return 需要忽略的挂钩点列表
     */
    public synchronized String[] getIgnoreHooks() {
        return this.ignoreHooks;
    }

    /**
     * 配置需要忽略的挂钩点
     *
     * @param ignoreHooks
     */
    public synchronized void setIgnoreHooks(String ignoreHooks) {
        this.ignoreHooks = ignoreHooks.replace(" ", "").split(",");
    }

    /**
     * 反射hook点传递给插件栈信息的最大深度
     *
     * @return 栈信息最大深度
     */
    public synchronized int getPluginMaxStack() {
        return pluginMaxStack;
    }

    /**
     * 设置反射hook点传递给插件栈信息的最大深度
     *
     * @param pluginMaxStack 栈信息最大深度
     */
    public synchronized void setPluginMaxStack(String pluginMaxStack) {
        this.pluginMaxStack = Integer.parseInt(pluginMaxStack);
        if (this.pluginMaxStack < 0) {
            this.pluginMaxStack = 0;
        }
    }

    /**
     * 获取反射监控的方法
     *
     * @return 需要监控的反射方法
     */
    public synchronized String[] getReflectionMonitorMethod() {
        return reflectionMonitorMethod;
    }

    /**
     * 设置反射监控的方法
     *
     * @param reflectionMonitorMethod 监控的方法
     */
    public synchronized void setReflectionMonitorMethod(String reflectionMonitorMethod) {
        this.reflectionMonitorMethod = reflectionMonitorMethod.replace(" ", "").split(",");
    }

    /**
     * 获取拦截自定义页面的url
     *
     * @return 拦截页面url
     */
    public synchronized String getBlockUrl() {
        return blockUrl;
    }

    /**
     * 设置拦截页面url
     *
     * @param blockUrl 拦截页面url
     */
    public synchronized void setBlockUrl(String blockUrl) {
        this.blockUrl = StringUtils.isEmpty(blockUrl) ? Item.BLOCK_URL.defaultValue : blockUrl;
    }

    /**
     * 获取报警日志最大输出栈深度
     *
     * @return
     */
    public synchronized int getLogMaxStackSize() {
        return logMaxStackSize;
    }

    /**
     * 配置报警日志最大输出栈深度
     *
     * @param logMaxStackSize
     */
    public synchronized void setLogMaxStackSize(String logMaxStackSize) {
        this.logMaxStackSize = Integer.parseInt(logMaxStackSize);
        if (this.logMaxStackSize < 0) {
            this.logMaxStackSize = 0;
        }
    }

    /**
     * 获取允许传入插件的ognl表达式的最短长度
     *
     * @return ognl表达式最短长度
     */
    public synchronized int getOgnlMinLength() {
        return ognlMinLength;
    }

    /**
     * 配置允许传入插件的ognl表达式的最短长度
     *
     * @param ognlMinLength ognl表达式最短长度
     */
    public synchronized void setOgnlMinLength(String ognlMinLength) {
        this.ognlMinLength = Integer.parseInt(ognlMinLength);
        if (this.ognlMinLength < 0) {
            this.ognlMinLength = 0;
        }
    }

    /**
     * 是否开启强制安全规范
     * 如果开启检测有安全风险的情况下将会禁止服务器启动
     * 如果关闭当有安全风险的情况下通过日志警告
     *
     * @return true开启，false关闭
     */
    public synchronized boolean getEnforcePolicy() {
        return enforcePolicy;
    }

    /**
     * 配置是否开启强制安全规范
     *
     * @return true开启，false关闭
     */
    public synchronized void setEnforcePolicy(String enforcePolicy) {
        this.enforcePolicy = Boolean.parseBoolean(enforcePolicy);
    }

    /**
     * 获取读文件需要检测的扩展名正则表达式
     *
     * @return
     */
    public synchronized String getReadFileExtensionRegex() {
        return readFileExtensionRegex;
    }

    /**
     * 设置读文件需要检测的扩展名正则表达式
     *
     * @param readFileExtensionRegex
     */
    public synchronized void setReadFileExtensionRegex(String readFileExtensionRegex) {
        this.readFileExtensionRegex = readFileExtensionRegex;
    }

    /**
     * 获取拦截状态码
     *
     * @return 状态码
     */
    public synchronized int getBlockStatusCode() {
        return blockStatusCode;
    }

    /**
     * 设置拦截状态码
     *
     * @param blockStatusCode 状态码
     */
    public synchronized void setBlockStatusCode(String blockStatusCode) {
        this.blockStatusCode = Integer.parseInt(blockStatusCode);
        if (this.blockStatusCode < 100 || this.blockStatusCode > 999) {
            this.blockStatusCode = 302;
        }
    }

    /**
     * 获取 debugLevel 级别
     * 0是关闭，非0开启
     *
     * @return debugLevel 级别
     */
    public synchronized int getDebugLevel() {
        return debugLevel;
    }

    /**
     * 设置 debugLevel 级别
     *
     * @param debugLevel debugLevel 级别
     */
    public synchronized void setDebugLevel(String debugLevel) {
        this.debugLevel = Integer.parseInt(debugLevel);
        if (this.debugLevel < 0) {
            this.debugLevel = 0;
        } else if (this.debugLevel > 0) {
            String debugEnableMessage = "[OpenRASP] Debug output enabled, debug_level=" + debugLevel;
            System.out.println(debugEnableMessage);
            LOGGER.info(debugEnableMessage);
        }
    }

    /**
     * 获取检测算法配置
     *
     * @return 配置的 json 对象
     */
    public synchronized JsonObject getAlgorithmConfig() {
        return algorithmConfig;
    }

    /**
     * 设置检测算法配置
     *
     * @param json 配置内容
     */
    public synchronized void setAlgorithmConfig(String json) {
        this.algorithmConfig = new JsonParser().parse(json).getAsJsonObject();
    }

    //--------------------------统一的配置处理------------------------------------

    /**
     * 统一配置接口,通过 js 更改配置的入口
     *
     * @param key   配置名
     * @param value 配置值
     * @return 是否配置成功
     */
    public boolean setConfig(String key, String value, boolean isInit) {
        try {
            boolean isHit = true;
            if (Item.BLOCK_URL.key.equals(key)) {
                setBlockUrl(value);
            } else if (Item.BODY_MAX_BYTES.key.equals(key)) {
                setBodyMaxBytes(value);
            } else if (Item.HOOKS_IGNORE.key.equals(key)) {
                setIgnoreHooks(value);
            } else if (Item.INJECT_URL_PREFIX.key.equals(key)) {
                setInjectUrlPrefix(value);
            } else if (Item.LOG_MAX_STACK.key.equals(key)) {
                setLogMaxStackSize(value);
            } else if (Item.OGNL_EXPRESSION_MIN_LENGTH.key.equals(key)) {
                setOgnlMinLength(value);
            } else if (Item.PLUGIN_TIMEOUT_MILLIS.key.equals(key)) {
                setPluginTimeout(value);
            } else if (Item.READ_FILE_EXTENSION_REGEX.key.equals(key)) {
                setReadFileExtensionRegex(value);
            } else if (Item.REFLECTION_MAX_STACK.key.equals(key)) {
                setPluginMaxStack(value);
            } else if (Item.SECURITY_ENFORCE_POLICY.key.equals((key))) {
                setEnforcePolicy(value);
            } else if (Item.SQL_SLOW_QUERY_MIN_ROWS.key.equals(key)) {
                setSqlSlowQueryMinCount(value);
            } else if (Item.BLOCK_STATUS_CODE.key.equals(key)) {
                setBlockStatusCode(value);
            } else if (Item.DEBUG.key.equals(key)) {
                setDebugLevel(value);
            } else if (Item.ALGORITHM_CONFIG.key.equals(key)) {
                setAlgorithmConfig(value);
            } else {
                isHit = false;
            }
            if (isHit) {
                if (isInit) {
                    LOGGER.info(key + ": " + value);
                } else {
                    LOGGER.info("configuration item \"" + key + "\" changed to \"" + value + "\"");
                }
            } else {
                LOGGER.info("configuration item \"" + key + "\" doesn't exist");
                return false;
            }
        } catch (Exception e) {
            if (isInit) {
                // 初始化配置过程中,如果报错需要继续使用默认值执行
                throw new ConfigLoadException(e);
            }
            LOGGER.info("configuration item \"" + key + "\" failed to change to \"" + value + "\"" + " because:" + e.getMessage());
            return false;
        }
        return true;
    }

}
