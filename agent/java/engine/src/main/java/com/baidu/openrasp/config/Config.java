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

package com.baidu.openrasp.config;

import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.model.HookWhiteModel;
import com.baidu.openrasp.cloud.syslog.DynamicConfigAppender;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.exceptions.ConfigLoadException;
import com.baidu.openrasp.messaging.LogConfig;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.tool.FileUtil;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.filemonitor.FileScanListener;
import com.baidu.openrasp.tool.filemonitor.FileScanMonitor;
import com.fuxi.javaagent.contentobjects.jnotify.JNotifyException;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonPrimitive;
import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;
import org.yaml.snakeyaml.Yaml;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;


/**
 * Created by tyy on 3/27/17.
 * 项目配置类，通过解析conf/rasp.property文件来加载配置
 * 若没有找到配置文件使用默认值
 */
public class Config extends FileScanListener {

    public enum Item {
        PLUGIN_TIMEOUT_MILLIS("plugin.timeout.millis", "100"),
        HOOKS_IGNORE("hooks.ignore", ""),
        INJECT_URL_PREFIX("inject.urlprefix", ""),
        REQUEST_PARAM_ENCODING("request.param_encoding", ""),
        BODY_MAX_BYTES("body.maxbytes", "4096"),
        LOG_MAX_STACK("log.maxstack", "50"),
        LOG_MAX_BACKUP("log.maxbackup", "30"),
        REFLECTION_MAX_STACK("plugin.maxstack", "100"),
        SQL_CACHE_CAPACITY("lru.max_size", "1024"),
        SECURITY_ENFORCE_POLICY("security.enforce_policy", "false"),
        PLUGIN_FILTER("plugin.filter", "true"),
        OGNL_EXPRESSION_MIN_LENGTH("ognl.expression.minlength", "30"),
        SQL_SLOW_QUERY_MIN_ROWS("sql.slowquery.min_rows", "500"),
        BLOCK_STATUS_CODE("block.status_code", "302"),
        DEBUG("debug.level", "0"),
        ALGORITHM_CONFIG("algorithm.config", "{}", false),
        CLIENT_IP_HEADER("clientip.header", "ClientIP"),
        BLOCK_REDIRECT_URL("block.redirect_url", "https://rasp.baidu.com/blocked/?request_id=%request_id%"),
        BLOCK_JSON("block.content_json", "{\"error\":true, \"reason\": \"Request blocked by OpenRASP\", \"request_id\": \"%request_id%\"}"),
        BLOCK_XML("block.content_xml", "<?xml version=\"1.0\"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>"),
        BLOCK_HTML("block.content_html", "</script><script>location.href=\"https://rasp.baidu.com/blocked2/?request_id=%request_id%\"</script>"),
        CLOUD_SWITCH("cloud.enable", "false"),
        CLOUD_ADDRESS("cloud.backend_url", ""),
        CLOUD_APPID("cloud.app_id", ""),
        CLOUD_APPSECRET("cloud.app_secret", ""),
        SYSLOG_ENABLE("syslog.enable", "false"),
        SYSLOG_URL("syslog.url", ""),
        SYSLOG_TAG("syslog.tag", "OPENRASP"),
        SYSLOG_FACILITY("syslog.facility", "1"),
        SYSLOG_RECONNECT_INTERVAL("syslog.reconnect_interval", "300000"),
        LOG_MAXBURST("log.maxburst", "100"),
        HEARTBEAT_INTERVAL("cloud.heartbeat_interval", "90"),
        HOOK_WHITE("hook.white", ""),
        HOOK_WHITE_ALL("hook.white.ALL", "true"),
        DECOMPILE_ENABLE("decompile.enable", "false"),
        RESPONSE_HEADERS("inject.custom_headers", "");


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

    private static final String HOOKS_WHITE = "hook.white";
    private static final String RESPONSE_HEADERS = "inject.custom_headers";
    private static final String CONFIG_DIR_NAME = "conf";
    private static final String CONFIG_FILE_NAME = "openrasp.yml";
    public static final int REFLECTION_STACK_START_INDEX = 0;
    public static final Logger LOGGER = Logger.getLogger(Config.class.getName());
    public static String baseDirectory;
    private static Integer watchId;
    //全局lru的缓存
    public static LRUCache<Object, String> commonLRUCache;

    private String configFileDir;
    private int pluginMaxStack;
    private long pluginTimeout;
    private int bodyMaxBytes;
    private int sqlSlowQueryMinCount;
    private String[] ignoreHooks;
    private boolean enforcePolicy;
    private String[] reflectionMonitorMethod;
    private int logMaxStackSize;
    private String blockUrl;
    private String injectUrlPrefix;
    private String requestParamEncoding;
    private int ognlMinLength;
    private int blockStatusCode;
    private int debugLevel;
    private JsonObject algorithmConfig;
    private String blockJson;
    private String blockXml;
    private String blockHtml;
    private boolean pluginFilter;
    private String clientIp;
    private boolean cloudSwitch;
    private String cloudAddress;
    private String cloudAppId;
    private String cloudAppSecret;
    private int sqlCacheCapacity;
    private boolean syslogSwitch;
    private String syslogUrl;
    private String syslogTag;
    private int syslogReconnectInterval;
    private boolean hookWhiteAll;
    private int logMaxBurst;
    private int heartbeatInterval;
    private int syslogFacility;
    private boolean decompileEnable;
    private Map<String, String> responseHeaders;
    private int logMaxBackUp;


    static {
        baseDirectory = FileUtil.getBaseDir();
        if (!getConfig().getCloudSwitch()) {
            CustomResponseHtml.load(baseDirectory);
        }
        //初始化全局缓存
        commonLRUCache = new LRUCache<Object, String>(getConfig().getSqlCacheCapacity());
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
            if (!getCloudSwitch()) {
                try {
                    FileScanMonitor.addMonitor(
                            baseDirectory, ConfigHolder.instance);
                } catch (JNotifyException e) {
                    throw new ConfigLoadException("add listener on " + baseDirectory + " failed because:" + e.getMessage());
                }
                addConfigFileMonitor();
            }
        } catch (FileNotFoundException e) {
            handleException("Could not find openrasp.yml, using default settings: " + e.getMessage(), e);
        } catch (JNotifyException e) {
            handleException("add listener on " + configFileDir + " failed because:" + e.getMessage(), e);
        } catch (IOException e) {
            handleException("cannot load properties file: " + e.getMessage(), e);
        }
    }

    @SuppressWarnings({"unchecked"})
    private synchronized void loadConfigFromFile(File file, boolean isInit) throws IOException {
        Map<String, Object> properties = null;
        try {
            if (file.exists()) {
                Yaml yaml = new Yaml();
                properties = yaml.loadAs(new FileInputStream(file), Map.class);
            }
        } catch (Exception e) {
            String message = "openrasp.yml parsing failed";
            int errorCode = ErrorType.CONFIG_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        } finally {
            TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
            // 出现解析问题使用默认值
            for (Config.Item item : Config.Item.values()) {
                if (item.key.equals(HOOKS_WHITE)) {
                    if (properties != null) {
                        Object object = properties.get(item.key);
                        if (object instanceof Map) {
                            Map<String, Object> hooks = (Map<String, Object>) object;
                            temp.putAll(parseHookWhite(hooks));
                        }
                    }
                    HookWhiteModel.init(temp);
                    continue;
                }
                if (item.key.equals(RESPONSE_HEADERS)) {
                    if (properties != null) {
                        Object object = properties.get(item.key);
                        if (object instanceof Map) {
                            Map<String, String> headers = (Map<String, String>) object;
                            setResponseHeaders(headers);
                        }
                    }
                    continue;
                }
                if (item.isProperties) {
                    setConfigFromProperties(item, properties, isInit);
                }
            }
        }
    }

    @SuppressWarnings("unchecked")
    public synchronized void loadConfigFromCloud(Map<String, Object> configMap, boolean isInit) {
        TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
        for (Map.Entry<String, Object> entry : configMap.entrySet()) {
            //开启云控必须参数不能云控
            if (entry.getKey().startsWith("cloud.")) {
                continue;
            }
            if (entry.getKey().equals(HOOKS_WHITE)) {
                if (entry.getValue() instanceof JsonObject) {
                    Map<String, Object> hooks = CloudUtils.getMapGsonObject().fromJson((JsonObject) entry.getValue(), Map.class);
                    temp.putAll(parseHookWhite(hooks));
                }
            } else if (entry.getKey().equals(RESPONSE_HEADERS)) {
                if (entry.getValue() instanceof JsonObject) {
                    Map<String, String> headers = CloudUtils.getMapGsonObject().fromJson((JsonObject) entry.getValue(), Map.class);
                    setResponseHeaders(headers);
                }
            } else {
                if (entry.getValue() instanceof JsonPrimitive) {
                    setConfig(entry.getKey(), ((JsonPrimitive) entry.getValue()).getAsString(), isInit);
                }
            }
        }
        HookWhiteModel.init(temp);
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
                    //更新log4j的日志限速
                    DynamicConfigAppender.fileAppenderAddBurstFilter();
                    //更新log4j的日志最大备份天数
                    DynamicConfigAppender.setLogMaxBackup();
                }
            } catch (IOException e) {
                String message = "update openrasp.yml failed";
                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
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

    private void setConfigFromProperties(Config.Item item, Map<String, Object> properties, boolean isInit) {
        String key = item.key;
        String value = item.defaultValue;
        if (properties != null) {
            Object object = properties.get(item.key);
            if (object != null) {
                value = String.valueOf(object);
            }
        }
        try {
            setConfig(key, value, isInit);
        } catch (Exception e) {
            // 出现解析问题使用默认值
            value = item.defaultValue;
            setConfig(key, item.defaultValue, false);
            String message = "set config " + item.key + " failed, use default value : " + value;
            int errorCode = ErrorType.CONFIG_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }
    }

    private void handleException(String message, Exception e) {
        int errorCode = ErrorType.CONFIG_ERROR.getCode();
        LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
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
            String message = "update " + directory.getAbsolutePath() + " failed";
            int errorCode = ErrorType.CONFIG_ERROR.getCode();
            LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
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
     * 配置Js引擎执行超时时间
     *
     * @param pluginTimeout 超时时间
     */
    public synchronized void setPluginTimeout(String pluginTimeout) {
        this.pluginTimeout = Long.parseLong(pluginTimeout);
        if (this.pluginTimeout <= 0) {
            this.pluginTimeout = 100;
        }
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
    public int getBodyMaxBytes() {
        return bodyMaxBytes;
    }

    /**
     * 配置保存HTTP请求体时最大保存长度
     *
     * @param bodyMaxBytes
     */
    public synchronized void setBodyMaxBytes(String bodyMaxBytes) {
        this.bodyMaxBytes = Integer.parseInt(bodyMaxBytes);
        if (this.bodyMaxBytes <= 0) {
            this.bodyMaxBytes = 4096;
        }
    }

    public int getSqlSlowQueryMinCount() {
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
    public String[] getIgnoreHooks() {
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
    public int getPluginMaxStack() {
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
            this.pluginMaxStack = 100;
        }
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
    public String getBlockUrl() {
        return blockUrl;
    }

    /**
     * 设置拦截页面url
     *
     * @param blockUrl 拦截页面url
     */
    public synchronized void setBlockUrl(String blockUrl) {
        this.blockUrl = StringUtils.isEmpty(blockUrl) ? Item.BLOCK_REDIRECT_URL.defaultValue : blockUrl;
    }

    /**
     * 获取报警日志最大输出栈深度
     *
     * @return
     */
    public int getLogMaxStackSize() {
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
            this.logMaxStackSize = 50;
        }
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
     * 配置允许传入插件的ognl表达式的最短长度
     *
     * @param ognlMinLength ognl表达式最短长度
     */
    public synchronized void setOgnlMinLength(String ognlMinLength) {
        this.ognlMinLength = Integer.parseInt(ognlMinLength);
        if (this.ognlMinLength <= 0) {
            this.ognlMinLength = 30;
        }
    }

    /**
     * 是否开启强制安全规范
     * 如果开启检测有安全风险的情况下将会禁止服务器启动
     * 如果关闭当有安全风险的情况下通过日志警告
     *
     * @return true开启，false关闭
     */
    public boolean getEnforcePolicy() {
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
     * 获取拦截状态码
     *
     * @return 状态码
     */
    public int getBlockStatusCode() {
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
    public int getDebugLevel() {
        return debugLevel;
    }

    /**
     * 是否开启调试
     *
     * @return true 代表开启
     */
    public synchronized boolean isDebugEnabled() {
        return debugLevel > 0;
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
    public JsonObject getAlgorithmConfig() {
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

    /**
     * 获取请求参数编码
     *
     * @return 请求参数编码
     */
    public String getRequestParamEncoding() {
        return requestParamEncoding;
    }

    /**
     * 设置请求参数编码
     * 当该配置不为空的情况下，将会允许 hook 点（如 request hook 点）能够根据设置的编码先于用户获取参数
     * （注意：如果设置了该编码，那么所有的请求参数都将按照这个编码解码，如果用户对参数有多种编码，建议不要添加此配置）
     * 当该配置为空的情况下，只有用户获取了参数之后，才会允许 hook 点获取参数，从而防止乱码问题
     *
     * @param requestParamEncoding 请求参数编码
     */
    public synchronized void setRequestParamEncoding(String requestParamEncoding) {
        this.requestParamEncoding = requestParamEncoding;
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
     * 设置响应的ContentType类型
     *
     * @param blockJson ContentType
     */
    public synchronized void setBlockJson(String blockJson) {
        this.blockJson = blockJson;
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
     * 设置响应的ContentType类型
     *
     * @param blockXml ContentType类型
     */
    public synchronized void setBlockXml(String blockXml) {
        this.blockXml = blockXml;
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
     * 设置响应的ContentType类型
     *
     * @param blockHtml ContentType
     */
    public synchronized void setBlockHtml(String blockHtml) {
        this.blockHtml = blockHtml;
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
     * 设置对于文件的include/reaFile等hook点，当文件不存在时，
     * 是否调用插件的开关状态
     *
     * @param pluginFilter 开关状态:on/off
     */
    public synchronized void setPluginFilter(String pluginFilter) {
        this.pluginFilter = Boolean.parseBoolean(pluginFilter);
    }

    /**
     * 获取自定义的请求头，
     *
     * @return 返回请求头
     */
    public String getClientIp() {
        return clientIp;
    }

    /**
     * 设置自定义的请求头，
     *
     * @param clientIp 待设置的请求头信息
     */
    public synchronized void setClientIp(String clientIp) {
        this.clientIp = clientIp;
    }

    /**
     * 获取sql的lruCache的大小，
     *
     * @return 缓存的大小
     */
    public int getSqlCacheCapacity() {
        return sqlCacheCapacity;
    }

    /**
     * 设置sql的lruCache的大小，
     *
     * @param sqlCacheCapacity 待设置的缓存大小，默认大小为100
     */
    public synchronized void setSqlCacheCapacity(String sqlCacheCapacity) {
        this.sqlCacheCapacity = Integer.parseInt(sqlCacheCapacity);
        if (this.sqlCacheCapacity < 0) {
            this.sqlCacheCapacity = 1024;
        }
        if (Config.commonLRUCache == null || Config.commonLRUCache.maxSize() != this.sqlCacheCapacity) {
            if (Config.commonLRUCache == null) {
                Config.commonLRUCache = new LRUCache<Object, String>(this.sqlCacheCapacity);
            } else {
                Config.commonLRUCache.clear();
                Config.commonLRUCache = new LRUCache<Object, String>(this.sqlCacheCapacity);
            }
        }
    }

    /**
     * 获取是否启用syslog开关状态，
     *
     * @return syslog开关状态
     */
    public boolean getSyslogSwitch() {
        return syslogSwitch;
    }

    /**
     * 设置syslog开关状态，
     *
     * @param syslogSwitch 待设置的syslog开关状态
     */
    public synchronized void setSyslogSwitch(String syslogSwitch) {
        this.syslogSwitch = Boolean.parseBoolean(syslogSwitch);
    }

    /**
     * 获取syslog上传日志的地址，
     *
     * @return syslog上传日志的地址
     */
    public String getSyslogUrl() {
        return syslogUrl;
    }

    /**
     * 设置syslog上传日志的地址，
     *
     * @param syslogUrl 待设置的syslog上传日志的地址
     */
    public synchronized void setSyslogUrl(String syslogUrl) {
        this.syslogUrl = syslogUrl;
    }

    /**
     * 获取syslog的layout中的tag字段信息，
     *
     * @return syslog的layout中的tag字段信息
     */
    public String getSyslogTag() {
        return syslogTag;
    }

    /**
     * 设置syslog的layout中的tag字段信息，
     *
     * @param syslogTag 待设置syslog的layout中的tag字段信息
     */
    public synchronized void setSyslogTag(String syslogTag) {
        this.syslogTag = syslogTag;
    }

    /**
     * 获取syslog的facility字段信息
     *
     * @return syslog的facility字段信息
     */
    public int getSyslogFacility() {
        return syslogFacility;
    }

    /**
     * 设置syslog的facility字段信息，
     *
     * @param syslogFacility 待设置syslog的facility字段信息
     */
    public synchronized void setSyslogFacility(String syslogFacility) {
        this.syslogFacility = Integer.parseInt(syslogFacility);
        if (!(this.syslogFacility >= 0 && this.syslogFacility <= 23)) {
            this.syslogFacility = 1;
        }
    }

    /**
     * 获取syslog的重连时间，
     *
     * @return syslog的重连时间
     */
    public int getSyslogReconnectInterval() {
        return syslogReconnectInterval;
    }

    /**
     * 设置syslog的重连时间，
     *
     * @param syslogReconnectInterval 待设置syslog的重连时间
     */
    public synchronized void setSyslogReconnectInterval(String syslogReconnectInterval) {
        this.syslogReconnectInterval = Integer.parseInt(syslogReconnectInterval);
        if (this.syslogReconnectInterval <= 0) {
            this.syslogReconnectInterval = 300000;
        }
    }

    /**
     * 获取日志每分钟上传的条数，
     *
     * @return 日志每分钟上传的条数
     */
    public int getLogMaxBurst() {
        return logMaxBurst;
    }

    /**
     * 设置日志每分钟上传的条数，
     *
     * @param logMaxBurst 待设置日志每分钟上传的条数
     */
    public synchronized void setLogMaxBurst(String logMaxBurst) {
        this.logMaxBurst = Integer.parseInt(logMaxBurst);
        if (this.logMaxBurst < 0) {
            this.logMaxBurst = 100;
        }
    }

    /**
     * 获取是否禁用全部hook点，
     *
     * @return 是否禁用全部hook点
     */
    public boolean getHookWhiteAll() {
        return hookWhiteAll;
    }

    /**
     * 设置是否禁用全部hook点，
     *
     * @param hookWhiteAll 是否禁用全部hook点
     */
    public synchronized void setHookWhiteAll(String hookWhiteAll) {
        this.hookWhiteAll = Boolean.parseBoolean(hookWhiteAll);
    }

    /**
     * 获取云控的开关状态，
     *
     * @return 云控开关状态
     */
    public boolean getCloudSwitch() {
        return cloudSwitch;
    }

    /**
     * 设置云控的开关状态，
     *
     * @param cloudSwitch 待设置的云控开关状态
     */
    public synchronized void setCloudSwitch(String cloudSwitch) {
        this.cloudSwitch = Boolean.parseBoolean(cloudSwitch);
    }

    /**
     * 获取云控地址，
     *
     * @return 返回云控地址
     */
    public String getCloudAddress() {
        return cloudAddress;
    }

    /**
     * 设置云控的地址，
     *
     * @param cloudAddress 待设置的云控地址
     */
    public synchronized void setCloudAddress(String cloudAddress) {
        this.cloudAddress = cloudAddress;
    }

    /**
     * 获取云控的请求的appid，
     *
     * @return 云控的请求的appid
     */
    public String getCloudAppId() {
        return cloudAppId;
    }

    /**
     * 设置云控的appid，
     *
     * @param cloudAppId 待设置的云控的appid
     */
    public synchronized void setCloudAppId(String cloudAppId) {
        this.cloudAppId = cloudAppId;
    }

    /**
     * 获取云控的请求的appSecret，
     *
     * @return 云控的请求的appSecret
     */
    public String getCloudAppSecret() {
        return cloudAppSecret;
    }

    /**
     * 设置云控的appSecret，
     *
     * @param cloudAppSecret 待设置的云控的appSecret
     */
    public synchronized void setCloudAppSecret(String cloudAppSecret) {
        this.cloudAppSecret = cloudAppSecret;
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
     * 设置云控的心跳请求间隔
     *
     * @param heartbeatInterval 待设置的云控心跳请求间隔
     */
    public synchronized void setHeartbeatInterval(String heartbeatInterval) {
        this.heartbeatInterval = Integer.parseInt(heartbeatInterval);
        if (!(this.heartbeatInterval >= 60 && this.heartbeatInterval <= 1800)) {
            this.heartbeatInterval = 180;
        }
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
     * 设置java反编译的开关状态
     *
     * @param decompileEnable 待设置java反编译的开关状态
     */
    public synchronized void setDecompileEnable(String decompileEnable) {
        this.decompileEnable = Boolean.parseBoolean(decompileEnable);
    }

    /**
     * 获取response header数组
     *
     * @return response header数组
     */
    public Map<String, String> getResponseHeaders() {
        return responseHeaders;
    }

    /**
     * 设置response header数组
     *
     * @param responseHeaders 待设置response header数组
     */
    public synchronized void setResponseHeaders(Map<String, String> responseHeaders) {
        this.responseHeaders = responseHeaders;
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
     * 设置log4j最大日志备份天数,默认30天
     *
     * @param logMaxBackUp log4j最大日志备份天数
     */
    public synchronized void setLogMaxBackUp(String logMaxBackUp) {
        this.logMaxBackUp = Integer.parseInt(logMaxBackUp) + 1;
        if (this.logMaxBackUp <= 0) {
            this.logMaxBackUp = 30;
        }
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
            if (Item.BLOCK_REDIRECT_URL.key.equals(key)) {
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
            } else if (Item.REQUEST_PARAM_ENCODING.key.equals(key)) {
                setRequestParamEncoding(value);
            } else if (Item.BLOCK_JSON.key.equals(key)) {
                setBlockJson(value);
            } else if (Item.BLOCK_XML.key.equals(key)) {
                setBlockXml(value);
            } else if (Item.BLOCK_HTML.key.equals(key)) {
                setBlockHtml(value);
            } else if (Item.PLUGIN_FILTER.key.equals(key)) {
                setPluginFilter(value);
            } else if (Item.CLIENT_IP_HEADER.key.equals(key)) {
                setClientIp(value);
            } else if (Item.CLOUD_SWITCH.key.equals(key)) {
                setCloudSwitch(value);
            } else if (Item.CLOUD_ADDRESS.key.equals(key)) {
                setCloudAddress(value);
            } else if (Item.CLOUD_APPID.key.equals(key)) {
                setCloudAppId(value);
            } else if (Item.CLOUD_APPSECRET.key.equals(key)) {
                setCloudAppSecret(value);
            } else if (Item.SQL_CACHE_CAPACITY.key.equals(key)) {
                setSqlCacheCapacity(value);
            } else if (Item.SYSLOG_ENABLE.key.equals(key)) {
                setSyslogSwitch(value);
            } else if (Item.SYSLOG_URL.key.equals(key)) {
                setSyslogUrl(value);
            } else if (Item.SYSLOG_TAG.key.equals(key)) {
                setSyslogTag(value);
            } else if (Item.SYSLOG_FACILITY.key.equals(key)) {
                setSyslogFacility(value);
            } else if (Item.SYSLOG_RECONNECT_INTERVAL.key.equals(key)) {
                setSyslogReconnectInterval(value);
            } else if (Item.LOG_MAXBURST.key.equals(key)) {
                setLogMaxBurst(value);
            } else if (Item.HEARTBEAT_INTERVAL.key.equals(key)) {
                setHeartbeatInterval(value);
            } else if (Item.DECOMPILE_ENABLE.key.equals(key)) {
                setDecompileEnable(value);
            } else if (Item.LOG_MAX_BACKUP.key.equals(key)) {
                setLogMaxBackUp(value);
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

    private TreeMap<String, Integer> parseHookWhite(Map<String, Object> hooks) {
        TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
        for (Map.Entry<String, Object> hook : hooks.entrySet()) {
            int codeSum = 0;
            if (hook.getValue() instanceof ArrayList) {
                @SuppressWarnings("unchecked")
                ArrayList<String> types = (ArrayList<String>) hook.getValue();
                if (hook.getKey().equals("*") && types.contains("all")) {
                    for (CheckParameter.Type type : CheckParameter.Type.values()) {
                        if (type.getCode() != 0) {
                            codeSum = codeSum + type.getCode();
                        }
                    }
                    temp.put("", codeSum);
                    return temp;
                } else if (types.contains("all")) {
                    for (CheckParameter.Type type : CheckParameter.Type.values()) {
                        if (type.getCode() != 0) {
                            codeSum = codeSum + type.getCode();
                        }
                    }
                    temp.put(hook.getKey(), codeSum);
                } else {
                    for (String s : types) {
                        String hooksType = s.toUpperCase();
                        try {
                            Integer code = CheckParameter.Type.valueOf(hooksType).getCode();
                            codeSum = codeSum + code;
                        } catch (Exception e) {
                            if (Config.getConfig().isDebugEnabled()) {
                                String message = "Hook type " + s + " does not exist";
                                int errorCode = ErrorType.CONFIG_ERROR.getCode();
                                LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
                            }
                        }
                    }
                    if (hook.getKey().equals("*")) {
                        temp.put("", codeSum);
                    } else {
                        temp.put(hook.getKey(), codeSum);
                    }
                }
            }
        }
        return temp;
    }
}
