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

package com.baidu.openrasp.config;

import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.exceptions.ConfigLoadException;
import com.baidu.openrasp.messaging.ErrorType;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.messaging.LogTool;
import com.baidu.openrasp.tool.FileUtil;
import com.baidu.openrasp.tool.FilterConstructor;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.google.gson.reflect.TypeToken;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;
import org.yaml.snakeyaml.Yaml;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.Map;
import java.util.Set;


/**
 * Created by tyy on 3/27/17.
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config extends FileScanListener {

    static final int MAX_SQL_EXCEPTION_CODES_COUNT = 100;
    static final int MAX_LOG_REGEX_COUNT = 20;
    static final int MAX_LOG_REGEX_LENGTH = 200;
    static final String CONFIG_DIR_NAME = "conf";
    static final String CONFIG_FILE_NAME = "openrasp.yml";
    public static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    public static String baseDirectory;
    static Integer watchId;
    //全局lru的缓存
    private static boolean isInit = false;
    public static LRUCache<Object, String> commonLRUCache;

    String configFileDir;
    int pluginMaxStack;
    long pluginTimeout;
    int bodyMaxBytes;
    int sqlSlowQueryMinCount;
    String[] ignoreHooks;
    String[] reflectionMonitorMethod;
    String blockUrl;
    String injectUrlPrefix;
    String requestParamEncoding;
    int ognlMinLength;
    int blockStatusCode;
    int debugLevel;
    JsonObject algorithmConfig;
    String blockJson;
    String blockXml;
    String blockHtml;
    boolean pluginFilter;
    String clientIp;
    boolean cloudSwitch;
    String cloudAddress;
    String cloudAppId;
    String cloudAppSecret;
    int sqlCacheCapacity;
    boolean syslogSwitch;
    String syslogUrl;
    String syslogTag;
    int syslogReconnectInterval;
    boolean hookWhiteAll;
    int logMaxBurst;
    int heartbeatInterval;
    int syslogFacility;
    boolean decompileEnable;
    Map<Object, Object> responseHeaders;
    int logMaxBackUp;
    int dependencyCheckInterval;
    List<String> securityWeakPasswords;
    boolean disableHooks;
    boolean cpuUsageEnable;
    int cpuUsagePercent;
    int cpuUsageCheckInterval;
    boolean isHttpsVerifyPeer;
    String raspId;
    Map<String, Set<String>> sqlErrorCodes;
    Map<String, Set<String>> sqlErrorStates;
    Map<String, String> logSensitiveRegex;
    boolean lruCompareEnable;
    int lruCompareLimit;
    int responseSamplerInterval;
    int responseSamplerBurst;
    boolean iastEnable;
    static Config instance;


    static {
        baseDirectory = FileUtil.getBaseDir();
        instance = new Config();
        instance.init();
        if (!instance.getCloudSwitch()) {
            CustomResponseHtml.load(baseDirectory);
        }
        isInit = true;
        //初始化全局缓存
        commonLRUCache = new LRUCache<Object, String>(instance.getSqlCacheCapacity());
        LOGGER.info("baseDirectory: " + baseDirectory);
    }

    /**
     * 构造函数，初始化全局配置
     */
    private Config() {
    }

    private void init() {
        this.configFileDir = baseDirectory + File.separator + CONFIG_DIR_NAME;
        String configFilePath = this.configFileDir + File.separator + CONFIG_FILE_NAME;
        try {
            loadConfigFromFile(new File(configFilePath), true);
            if (!getCloudSwitch()) {
                try {
                    FileScanMonitor.addMonitor(
                            baseDirectory, instance);
                } catch (JNotifyException e) {
                    throw new ConfigLoadException("add listener on " + baseDirectory + " failed because:" + e.getMessage());
                }
                addConfigFileMonitor();
            }
        } catch (FileNotFoundException e) {
            handleException("Could not find openrasp.yml, using default settings: " + e.getMessage(), e);
        } catch (JNotifyException e) {
            handleException("add listener on " + configFileDir + " failed because:" + e.getMessage(), e);
        } catch (Exception e) {
            handleException("cannot load properties file: " + e.getMessage(), e);
        }
        String configValidMsg = checkMajorConfig();
        if (configValidMsg != null) {
            LogTool.error(ErrorType.CONFIG_ERROR, configValidMsg);
            throw new ConfigLoadException(configValidMsg);
        }
    }

    /**
     * 检查关键配置选项
     */
    private String checkMajorConfig() {
        if (!StringUtils.isEmpty(raspId)) {
            if ((raspId.length() < 16 || raspId.length() > 512)) {
                return "the length of rasp.id must be between [16,512]";
            }
            for (int i = 0; i < raspId.length(); i++) {
                char a = raspId.charAt(i);
                if (!((a >= 'a' && a <= 'z') || (a >= '0' && a <= '9') || (a >= 'A' && a <= 'Z'))) {
                    return "the rasp.id can only contain letters and numbers";
                }
            }
        }
        return null;
    }

    @SuppressWarnings({"unchecked"})
    private synchronized void loadConfigFromFile(File file, boolean isInit) throws Exception {
        Map<String, Object> properties = null;
        try {
            if (file.exists()) {
                Yaml yaml = new Yaml(new FilterConstructor());
                properties = yaml.loadAs(new FileInputStream(file), Map.class);
                if (properties != null) {
                    for (String key : properties.keySet()) {
                        ConfigItem item = getConfigItem(key);
                        if (item == null) {
                            LogTool.warn(ErrorType.CONFIG_ERROR,
                                    "Unknown config key [" + key + "] found in yml configuration");
                        }
                    }
                }
            }
        } catch (Exception e) {
            LogTool.warn(ErrorType.CONFIG_ERROR, "openrasp.yml parsing failed: " + e.getMessage(), e);
        } finally {
            // 出现解析问题使用默认值
            for (ConfigItem item : ConfigItem.values()) {
                if (item.isProperties) {
                    setConfigFromProperties(item, properties, isInit);
                }
            }
        }
    }

    @SuppressWarnings("unchecked")
    public synchronized void loadConfigFromCloud(Map<String, Object> configMap, boolean isInit) throws Exception {
        for (Map.Entry<String, Object> entry : configMap.entrySet()) {
            ConfigItem item = getConfigItem(entry.getKey());
            if (item == null) {
                LogTool.warn(ErrorType.CONFIG_ERROR,
                        "Unknown config key [" + entry.getKey() + "] found in cloud configuration");
                continue;
            }
            // 无法云控的参数
            if (entry.getKey().startsWith("cloud.") || entry.getKey().equals("rasp.id")) {
                continue;
            }

            Object value = null;
            if (entry.getKey().equals(ConfigItem.HOOK_WHITE.toString())) {
                if (entry.getValue() instanceof JsonObject) {
                    value = CloudUtils.getMapGsonObject().fromJson((JsonObject) entry.getValue(), Map.class);
                }
            } else if (entry.getKey().equals(ConfigItem.RESPONSE_HEADERS.toString())) {
                if (entry.getValue() instanceof JsonObject) {
                    value = CloudUtils.getMapGsonObject().fromJson((JsonObject) entry.getValue(), Map.class);
                }
            } else if (entry.getKey().equals(ConfigItem.SECURITY_WEAK_PASSWORDS.toString())) {
                if (entry.getValue() instanceof JsonArray) {
                    value = new Gson().fromJson((JsonArray) entry.getValue(), new TypeToken<List<String>>() {
                    }.getType());
                }
            } else if (entry.getValue() instanceof JsonPrimitive) {
                value = ((JsonPrimitive) entry.getValue()).getAsString();
            }
            try {
                if (value != null) {
                    setConfig(item, value, isInit);
                }
            } catch (Exception e) {
                // 出现解析问题使用默认值
                String message = "Configuration item " + entry.getKey() + " has invalid value "
                        + entry.getValue() + ", using default: " + item.setter.getDefaultValue();
                LogTool.warn(ErrorType.CONFIG_ERROR, message + ", reason: " + e.getMessage(), e);
                setDefaultValue(item);
            }
        }
    }

    private void reloadConfig(File file) {
        if (file.getName().equals(CONFIG_FILE_NAME)) {
            try {
                loadConfigFromFile(file, false);
                //单机模式下动态添加获取删除syslog和动态更新syslog tag
                if (!CloudUtils.checkCloudControlEnter()) {
                    //关闭或者打开syslog服务
                    LogConfig.syslogManager();
                    //更新syslog tag标志
                    DynamicConfigAppender.updateSyslogTag();
                    //是否开启log4j的debug
                    DynamicConfigAppender.enableDebug();
                    //更新log4j appender 打印日志的路径
                    DynamicConfigAppender.updateLog4jPath(false, null);
                    //更新log4j的日志限速
                    DynamicConfigAppender.fileAppenderAddBurstFilter();
                    //更新log4j的日志最大备份天数
                    DynamicConfigAppender.setLogMaxBackup();
                }
            } catch (Exception e) {
                LogTool.warn(ErrorType.CONFIG_ERROR, "update openrasp.yml failed: " + e.getMessage(), e);
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

    private void setConfigFromProperties(ConfigItem item, Map<String, Object> properties, boolean isInit) throws Exception {
        String key = item.toString();
        Object value = null;
        if (properties != null) {
            value = properties.get(key);
        }
        if (value == null) {
            setDefaultValue(item);
        } else {
            try {
                setConfig(item, value, isInit);
            } catch (Exception e) {
                // 出现解析问题使用默认值
                String message = "set config " + item.toString() + " failed";
                LogTool.warn(ErrorType.CONFIG_ERROR, message + ", because: " + e.getMessage(), e);
                setDefaultValue(item);
            }
        }
    }

    private void setDefaultValue(ConfigItem item) {
        if (item.setter.setDefaultValue()) {
            LOGGER.info(item.toString() + " use the default value: " + item.setter.getDefaultValue());
        }
    }

    private void handleException(String message, Exception e) {
        LogTool.warn(ErrorType.CONFIG_ERROR, message, e);
    }

    /**
     * 获取配置单例
     *
     * @return Config单例对象
     */
    public static Config getConfig() {
        return instance;
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
            if (isInit) {
                if (directory.getName().equals(CustomResponseHtml.CUSTOM_RESPONSE_BASE_DIR)) {
                    CustomResponseHtml.load(baseDirectory);
                } else if (directory.getName().equals(CONFIG_DIR_NAME)) {
                    reloadConfig(new File(configFileDir + File.separator + CONFIG_FILE_NAME));
                }
            }
        } catch (Exception e) {
            LogTool.warn(ErrorType.CONFIG_ERROR, "update " + directory.getAbsolutePath() +
                    " failed: " + e.getMessage(), e);
        }
    }

    //--------------------可以通过插件修改的配置项-----------------------------------

    /**
     * 获取Js引擎执行超时时间
     *
     * @return 超时时间
     */
    public long getPluginTimeout() {
        return pluginTimeout;
    }

    /**
     * 设置需要插入自定义html的页面path前缀
     *
     * @return 页面path前缀
     */
    public String getInjectUrlPrefix() {
        return injectUrlPrefix;
    }

    /**
     * 保存HTTP请求体时最大保存长度
     *
     * @return 最大长度
     */
    public int getBodyMaxBytes() {
        return bodyMaxBytes;
    }

    public int getSqlSlowQueryMinCount() {
        return sqlSlowQueryMinCount;
    }

    /**
     * 需要忽略的挂钩点
     *
     * @return 需要忽略的挂钩点列表
     */
    public String[] getIgnoreHooks() {
        return this.ignoreHooks;
    }

    /**
     * 反射hook点传递给插件栈信息的最大深度
     *
     * @return 栈信息最大深度
     */
    public int getPluginMaxStack() {
        return pluginMaxStack;
    }

    /**
     * 获取反射监控的方法
     *
     * @return 需要监控的反射方法
     */
    public String[] getReflectionMonitorMethod() {
        return reflectionMonitorMethod;
    }

    /**
     * 获取敏感日志检测正则
     *
     * @return 正则信息
     */
    public Map<String, String> getLogSensitiveRegex() {
        return logSensitiveRegex;
    }

    /**
     * 获取 rasp id
     *
     * @return rasp id
     */
    public String getRaspId() {
        return raspId;
    }

    /**
     * 获取拦截自定义页面的url
     *
     * @return 拦截页面url
     */
    public String getBlockUrl() {
        return blockUrl;
    }

    /**
     * 获取允许传入插件的ognl表达式的最短长度
     *
     * @return ognl表达式最短长度
     */
    public int getOgnlMinLength() {
        return ognlMinLength;
    }

    /**
     * 获取拦截状态码
     *
     * @return 状态码
     */
    public int getBlockStatusCode() {
        return blockStatusCode;
    }

    /**
     * 获取 debugLevel 级别
     * 0是关闭，非0开启
     *
     * @return debugLevel 级别
     */
    public int getDebugLevel() {
        return debugLevel;
    }

    /**
     * 获取 LRU 内容匹配开关
     *
     * @return LRU 内容匹配开关
     */
    public boolean getLruCompareEnable() {
        return lruCompareEnable;
    }

    /**
     * 获取 LRU 匹配最长字节
     *
     * @return LRU 匹配最长字节
     */
    public int getLruCompareLimit() {
        return lruCompareLimit;
    }

    /**
     * 是否开启调试
     *
     * @return true 代表开启
     */
    public boolean isDebugEnabled() {
        return debugLevel > 0;
    }

    /**
     * 是否开启 iast
     *
     * @return true 代表开启
     */
    public boolean isIastEnabled() {
        return iastEnable;
    }

    /**
     * 获取检测算法配置
     *
     * @return 配置的 json 对象
     */
    public JsonObject getAlgorithmConfig() {
        return algorithmConfig;
    }

    /**
     * 获取 sql 异常检测过滤的 sql 错误码
     *
     * @return sql 错误码列表
     */
    public Map<String, Set<String>> getSqlErrorCodes() {
        return sqlErrorCodes;
    }

    /**
     * 获取 sql 异常检测过滤的 sql 状态码
     *
     * @return sql 状态码列表
     */
    public Map<String, Set<String>> getSqlErrorStates() {
        return sqlErrorStates;
    }

    /**
     * 获取请求参数编码
     *
     * @return 请求参数编码
     */
    public String getRequestParamEncoding() {
        return requestParamEncoding;
    }

    /**
     * 获取响应的contentType类型
     *
     * @return 返回contentType类型
     */
    public String getBlockJson() {
        return blockJson;
    }

    /**
     * 获取响应的contentType类型
     *
     * @return 返回contentType类型
     */
    public String getBlockXml() {
        return blockXml;
    }

    /**
     * 获取响应的contentType类型
     *
     * @return 返回contentType类型
     */
    public String getBlockHtml() {
        return blockHtml;
    }

    /**
     * 获取对于文件的include/reaFile等hook点，当文件不存在时，
     * 是否调用插件的开关状态
     *
     * @return 返回是否进入插件
     */
    public boolean getPluginFilter() {
        return pluginFilter;
    }

    /**
     * 获取自定义的请求头
     *
     * @return 返回请求头
     */
    public String getClientIp() {
        return clientIp;
    }

    /**
     * 获取sql的lruCache的大小
     *
     * @return 缓存的大小
     */
    public int getSqlCacheCapacity() {
        return sqlCacheCapacity;
    }

    /**
     * 获取是否启用syslog开关状态
     *
     * @return syslog开关状态
     */
    public boolean getSyslogSwitch() {
        return syslogSwitch;
    }

    /**
     * 获取syslog上传日志的地址
     *
     * @return syslog上传日志的地址
     */
    public String getSyslogUrl() {
        return syslogUrl;
    }

    /**
     * 获取syslog的layout中的tag字段信息
     *
     * @return syslog的layout中的tag字段信息
     */
    public String getSyslogTag() {
        return syslogTag;
    }

    /**
     * 获取 syslog 的 facility 字段信息
     *
     * @return syslog 的 facility 字段信息
     */
    public int getSyslogFacility() {
        return syslogFacility;
    }

    /**
     * 获取 syslog 的重连时间
     *
     * @return syslog 的重连时间
     */
    public int getSyslogReconnectInterval() {
        return syslogReconnectInterval;
    }

    /**
     * 获取日志每分钟上传的条数
     *
     * @return 日志每分钟上传的条数
     */
    public int getLogMaxBurst() {
        return logMaxBurst;
    }

    /**
     * 获取是否禁用全部 hook 点
     *
     * @return 是否禁用全部 hook 点
     */
    public boolean getHookWhiteAll() {
        return hookWhiteAll;
    }

    /**
     * 设置是否禁用全部 hook 点
     *
     * @param hookWhiteAll 是否禁用全部hook点
     */
    public synchronized void setHookWhiteAll(String hookWhiteAll) {
        this.hookWhiteAll = Boolean.parseBoolean(hookWhiteAll);
    }

    /**
     * 获取是否禁用全部 hook 点
     *
     * @return 是否禁用全部 hook 点
     */
    public boolean getDisableHooks() {
        return disableHooks;
    }

    /**
     * 设置是否禁用全部hook点，
     *
     * @param disableHooks 是否禁用全部hook点
     */
    public synchronized void setDisableHooks(String disableHooks) {
        this.disableHooks = Boolean.parseBoolean(disableHooks);
    }

    /**
     * 获取云控的开关状态
     *
     * @return 云控开关状态
     */
    public boolean getCloudSwitch() {
        return cloudSwitch;
    }

    /**
     * 获取云控地址
     *
     * @return 返回云控地址
     */
    public String getCloudAddress() {
        return cloudAddress;
    }

    /**
     * 获取云控的请求的 appid
     *
     * @return 云控的请求的 appid
     */
    public String getCloudAppId() {
        return cloudAppId;
    }

    /**
     * 获取云控的请求的 appSecret
     *
     * @return 云控的请求的 appSecret
     */
    public String getCloudAppSecret() {
        return cloudAppSecret;
    }

    /**
     * 获取云控的心跳请求间隔，
     *
     * @return 云控的心跳请求间隔
     */
    public int getHeartbeatInterval() {
        return heartbeatInterval;
    }

    /**
     * 获取java反编译的开关状态
     *
     * @return java反编译的开关状态
     */
    public boolean getDecompileEnable() {
        return decompileEnable;
    }

    /**
     * 获取response header数组
     *
     * @return response header数组
     */
    public Map<Object, Object> getResponseHeaders() {
        return responseHeaders;
    }

    /**
     * 获取log4j最大日志备份天数
     *
     * @return log4j最大日志备份天数
     */
    public int getLogMaxBackUp() {
        return logMaxBackUp;
    }

    /**
     * 获取dependencyChecker的上报时间间隔
     *
     * @return dependencyChecker的上报时间间隔
     */
    public int getDependencyCheckInterval() {
        return dependencyCheckInterval;
    }

    public List<String> getSecurityWeakPasswords() {
        return securityWeakPasswords;
    }

    /**
     * 获取agent是否开启cpu熔断策略
     *
     * @return 是否开启cpu熔断策略
     */
    public boolean getCpuUsageEnable() {
        return cpuUsageEnable;
    }

    /**
     * 获取熔断检测时间间隔，单位/秒
     *
     * @return 时间间隔
     */
    public int getCpuUsageCheckInterval() {
        return cpuUsageCheckInterval;
    }

    /**
     * 获取cpu的使用率的百分比
     *
     * @return cpu的使用率的百分比
     */
    public int getCpuUsagePercent() {
        return cpuUsagePercent;
    }

    /**
     * 获取是否进行 https 证书验证
     *
     * @return 是否进行 https 证书验证
     */
    public boolean isHttpsVerifyPeer() {
        return isHttpsVerifyPeer;
    }

    public int getResponseSamplerInterval() {
        return responseSamplerInterval;
    }

    public int getResponseSamplerBurst() {
        return responseSamplerBurst;
    }

    //--------------------------统一的配置处理------------------------------------

    /**
     * 统一配置接口,通过 js 更改配置的入口
     *
     * @param item  配置项
     * @param value 配置值
     * @return 是否配置成功
     */
    public boolean setConfig(ConfigItem item, Object value, boolean isInit) throws Exception {
        try {
            if (ConfigItem.RASP_ID.equals(item)) {
                if (!isInit && !value.equals(raspId)) {
                    LOGGER.info("can not update the value of rasp.id at runtime");
                    return false;
                }
            }
            if (!(value instanceof String) && !(value instanceof Map) && !(value instanceof List)) {
                value = String.valueOf(value);
            }
            item.setter.setValue(value);
            if (isInit) {
                LOGGER.info(item.toString() + ": " + String.valueOf(value));
            } else {
                LOGGER.info("configuration item \"" + item.toString() +
                        "\" changed to \"" + String.valueOf(value) + "\"");
            }
        } catch (Exception e) {
            if (!isInit) {
                LOGGER.warn("configuration item \"" + item.toString() + "\" failed to change to \"" + value + "\"" + " because:" + e.getMessage());
            }
            // 初始化配置过程中,如果报错需要继续使用默认值执行
            if (!(e instanceof ConfigLoadException)) {
                throw new ConfigLoadException(e);
            }
            throw e;
        }
        return true;
    }

    // config项赋值前检测key是否是rasp规定的字段
    public ConfigItem getConfigItem(String key) {
        if (!StringUtils.isEmpty(key)) {
            for (ConfigItem item : ConfigItem.values()) {
                if (item.toString().equals(key)) {
                    return item;
                }
            }
        }
        return null;
    }
}
