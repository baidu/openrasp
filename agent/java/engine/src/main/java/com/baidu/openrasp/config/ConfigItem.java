package com.baidu.openrasp.config;

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.HookWhiteModel;
import com.baidu.openrasp.exceptions.ConfigLoadException;
import com.baidu.openrasp.plugin.checker.local.ConfigurableChecker;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.cpumonitor.CpuMonitorManager;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import org.apache.commons.lang3.StringUtils;

import java.util.*;

/**
 * Created by tyy on 19-10-22.
 */
public enum ConfigItem {
    PLUGIN_TIMEOUT_MILLIS(new ConfigSetter<String>("plugin.timeout.millis") {
        @Override
        public synchronized void setValue(String timeout) {
            long value = Long.parseLong(timeout);
            if (value <= 0) {
                throw new ConfigLoadException(itemName + " must be greater than 0");
            }
            Config.getConfig().pluginTimeout = value;
        }

        @Override
        public String getDefaultValue() {
            return "100";
        }
    }),

    HOOKS_IGNORE(new ConfigSetter<String>("hooks.ignore") {
        @Override
        public synchronized void setValue(String ignoreHooks) {
            Config.getConfig().ignoreHooks = ignoreHooks.replace(" ", "").split(",");
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    INJECT_URL_PREFIX(new ConfigSetter<String>("inject.urlprefix") {
        @Override
        public synchronized void setValue(String injectUrlPrefix) {
            StringBuilder injectPrefix = new StringBuilder(injectUrlPrefix);
            while (injectPrefix.length() > 0 && injectPrefix.charAt(injectPrefix.length() - 1) == '/') {
                injectPrefix.deleteCharAt(injectPrefix.length() - 1);
            }
            Config.getConfig().injectUrlPrefix = injectPrefix.toString();
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    REQUEST_PARAM_ENCODING(new ConfigSetter<String>("request.param_encoding") {
        @Override
        public synchronized void setValue(String requestParamEncoding) {
            Config.getConfig().requestParamEncoding = requestParamEncoding;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    BODY_MAX_BYTES(new ConfigSetter<String>("body.maxbytes") {
        @Override
        public synchronized void setValue(String bodyMaxBytes) {
            int value = Integer.parseInt(bodyMaxBytes);
            if (value <= 0) {
                throw new ConfigLoadException(itemName + " must be greater than 0");
            }
            Config.getConfig().bodyMaxBytes = value;
        }

        @Override
        public String getDefaultValue() {
            return "12288";
        }
    }),

    LOG_MAX_BACKUP(new ConfigSetter<String>("log.maxbackup") {
        @Override
        public synchronized void setValue(String logMaxBackUp) {
            int value = Integer.parseInt(logMaxBackUp) + 1;
            if (value <= 0) {
                throw new ConfigLoadException(itemName + " can not be less than 0");
            }
            Config.getConfig().logMaxBackUp = value;
        }

        @Override
        public String getDefaultValue() {
            return "30";
        }
    }),

    PLUGIN_MAX_STACK(new ConfigSetter<String>("plugin.maxstack") {
        @Override
        public synchronized void setValue(String pluginMaxStack) {
            int value = Integer.parseInt(pluginMaxStack);
            if (value < 0) {
                throw new ConfigLoadException(itemName + " can not be less than 0");
            }
            Config.getConfig().pluginMaxStack = value;
        }

        @Override
        public String getDefaultValue() {
            return "100";
        }
    }),

    SQL_CACHE_CAPACITY(new ConfigSetter<String>("lru.max_size") {
        @Override
        public synchronized void setValue(String sqlCacheCapacity) {
            int value = Integer.parseInt(sqlCacheCapacity);
            if (value < 0) {
                throw new ConfigLoadException(itemName + " can not be less than 0");
            }
            Config.getConfig().sqlCacheCapacity = value;
            if (Config.commonLRUCache == null || Config.commonLRUCache.maxSize() != value) {
                if (Config.commonLRUCache == null) {
                    Config.commonLRUCache = new LRUCache<Object, String>(value);
                } else {
                    Config.commonLRUCache.clear();
                    Config.commonLRUCache = new LRUCache<Object, String>(value);
                }
            }
        }

        @Override
        public String getDefaultValue() {
            return "1024";
        }
    }),

    PLUGIN_FILTER(new ConfigSetter<String>("plugin.filter") {
        @Override
        public synchronized void setValue(String pluginFilter) {
            Config.getConfig().pluginFilter = Boolean.parseBoolean(pluginFilter);
        }

        @Override
        public String getDefaultValue() {
            return "true";
        }
    }),

    OGNL_EXPRESSION_MIN_LENGTH(new ConfigSetter<String>("ognl.expression.minlength") {
        @Override
        public synchronized void setValue(String ognlMinLength) {
            int value = Integer.parseInt(ognlMinLength);
            if (value <= 0) {
                throw new ConfigLoadException(itemName + " must be greater than 0");
            }
            Config.getConfig().ognlMinLength = value;
        }

        @Override
        public String getDefaultValue() {
            return "30";
        }
    }),

    SQL_SLOW_QUERY_MIN_ROWS(new ConfigSetter<String>("sql.slowquery.min_rows") {
        @Override
        public synchronized void setValue(String sqlSlowQueryMinCount) {
            int value = Integer.parseInt(sqlSlowQueryMinCount);
            if (value < 0) {
                throw new ConfigLoadException(itemName + " can not be less than 0");
            }
            Config.getConfig().sqlSlowQueryMinCount = value;
        }

        @Override
        public String getDefaultValue() {
            return "500";
        }
    }),

    BLOCK_STATUS_CODE(new ConfigSetter<String>("block.status_code") {
        @Override
        public synchronized void setValue(String blockStatusCode) {
            int value = Integer.parseInt(blockStatusCode);
            if (value < 100 || value > 999) {
                throw new ConfigLoadException(itemName + " must be between [100,999]");
            }
            Config.getConfig().blockStatusCode = value;
        }

        @Override
        public String getDefaultValue() {
            return "302";
        }
    }),

    DEBUG(new ConfigSetter<String>("debug.level") {
        @Override
        public synchronized void setValue(String debugLevel) {
            Config.getConfig().debugLevel = Integer.parseInt(debugLevel);
            if (Config.getConfig().debugLevel < 0) {
                Config.getConfig().debugLevel = 0;
            } else if (Config.getConfig().debugLevel > 0) {
                String debugEnableMessage = "[OpenRASP] Debug output enabled, debug_level=" + debugLevel;
                System.out.println(debugEnableMessage);
                Config.LOGGER.info(debugEnableMessage);
            }
        }

        @Override
        public String getDefaultValue() {
            return "0";
        }
    }),

    ALGORITHM_CONFIG(new ConfigSetter<String>("algorithm.config") {
        @Override
        public synchronized void setValue(String json) {
            Config.getConfig().algorithmConfig = new JsonParser().parse(json).getAsJsonObject();
            try {
                JsonArray result = null;
                JsonElement elements = ConfigurableChecker.getElement(Config.getConfig().algorithmConfig,
                        "sql_exception", "mysql");
                if (elements != null) {
                    JsonElement e = elements.getAsJsonObject().get("error_code");
                    if (e != null) {
                        result = e.getAsJsonArray();
                    }
                }
                HashSet<Integer> errorCodes = new HashSet<Integer>();
                if (result != null) {
                    if (result.size() > Config.MAX_SQL_EXCEPTION_CODES_CONUT) {
                        Config.LOGGER.warn("size of RASP.algorithmConfig.sql_exception.error_code can not be greater than "
                                + Config.MAX_SQL_EXCEPTION_CODES_CONUT);
                    } else {
                        for (JsonElement element : result) {
                            try {
                                errorCodes.add(element.getAsInt());
                            } catch (Exception e) {
                                Config.LOGGER.warn("failed to add a json error code element: "
                                        + element.toString() + ", " + e.getMessage(), e);
                            }
                        }
                    }
                } else {
                    Config.LOGGER.warn("failed to get sql_exception.${DB_TYPE}.error_code from algorithm config");
                }
                Config.getConfig().sqlErrorCodes = errorCodes;
                Config.LOGGER.info("mysql sql error codes: " + Config.getConfig().sqlErrorCodes.toString());
            } catch (Exception e) {
                Config.LOGGER.warn("failed to get json error code element: " + e.getMessage(), e);
            }
        }

        @Override
        public String getDefaultValue() {
            return "{}";
        }
    }, false),

    CLIENT_IP_HEADER(new ConfigSetter<String>("clientip.header") {
        @Override
        public synchronized void setValue(String clientIp) {
            Config.getConfig().clientIp = clientIp;
        }

        @Override
        public String getDefaultValue() {
            return "ClientIP";
        }
    }),

    BLOCK_REDIRECT_URL(new ConfigSetter<String>("block.redirect_url") {
        @Override
        public synchronized void setValue(String blockUrl) {
            if (StringUtils.isEmpty(blockUrl)) {
                throw new ConfigLoadException(itemName + " can not be empty");
            }
            Config.getConfig().blockUrl = blockUrl;
        }

        @Override
        public String getDefaultValue() {
            return "https://rasp.baidu.com/blocked/?request_id=%request_id%";
        }
    }),

    BLOCK_JSON(new ConfigSetter<String>("block.content_json") {
        @Override
        public synchronized void setValue(String blockJson) {
            Config.getConfig().blockJson = blockJson;
        }

        @Override
        public String getDefaultValue() {
            return "{\"error\":true, \"reason\": \"Request blocked by OpenRASP\"," +
                    " \"request_id\": \"%request_id%\"}";
        }
    }),

    BLOCK_XML(new ConfigSetter<String>("block.content_xml") {
        @Override
        public synchronized void setValue(String blockXml) {
            Config.getConfig().blockXml = blockXml;
        }

        @Override
        public String getDefaultValue() {
            return "<?xml version=\"1.0\"?><doc><error>true</error>" +
                    "<reason>Request blocked by OpenRASP</reason>" +
                    "<request_id>%request_id%</request_id></doc>";
        }
    }),

    BLOCK_HTML(new ConfigSetter<String>("block.content_html") {
        @Override
        public synchronized void setValue(String blockHtml) {
            Config.getConfig().blockHtml = blockHtml;
        }

        @Override
        public String getDefaultValue() {
            return "</script><script>location.href=\"https://rasp.baidu.com/blocked2/" +
                    "?request_id=%request_id%\"</script>";
        }
    }),

    CLOUD_SWITCH(new ConfigSetter<String>("cloud.enable") {
        @Override
        public synchronized void setValue(String cloudSwitch) {
            Config.getConfig().cloudSwitch = Boolean.parseBoolean(cloudSwitch);
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    CLOUD_ADDRESS(new ConfigSetter<String>("cloud.backend_url") {
        @Override
        public synchronized void setValue(String cloudAddress) {
            Config.getConfig().cloudAddress = cloudAddress;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    CLOUD_APPID(new ConfigSetter<String>("cloud.app_id") {
        @Override
        public synchronized void setValue(String cloudAppId) {
            Config.getConfig().cloudAppId = cloudAppId;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    CLOUD_APPSECRET(new ConfigSetter<String>("cloud.app_secret") {
        @Override
        public synchronized void setValue(String cloudAppSecret) {
            Config.getConfig().cloudAppSecret = cloudAppSecret;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    RASP_ID(new ConfigSetter<String>("rasp.id") {
        @Override
        public synchronized void setValue(String raspId) {
            Config.getConfig().raspId = raspId;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    SYSLOG_ENABLE(new ConfigSetter<String>("syslog.enable") {
        @Override
        public synchronized void setValue(String syslogSwitch) {
            Config.getConfig().syslogSwitch = Boolean.parseBoolean(syslogSwitch);
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    SYSLOG_URL(new ConfigSetter<String>("syslog.url") {
        @Override
        public synchronized void setValue(String syslogUrl) {
            Config.getConfig().syslogUrl = syslogUrl;
        }

        @Override
        public String getDefaultValue() {
            return "";
        }
    }),

    SYSLOG_TAG(new ConfigSetter<String>("syslog.tag") {
        @Override
        public synchronized void setValue(String syslogTag) {
            Config.getConfig().syslogTag = syslogTag;
        }

        @Override
        public String getDefaultValue() {
            return "OPENRASP";
        }
    }),

    SYSLOG_FACILITY(new ConfigSetter<String>("syslog.facility") {
        @Override
        public synchronized void setValue(String syslogFacility) {
            int value = Integer.parseInt(syslogFacility);
            if (!(value >= 0 && value <= 23)) {
                throw new ConfigLoadException(itemName + " must be between [0,23]");
            }
            Config.getConfig().syslogFacility = value;
        }

        @Override
        public String getDefaultValue() {
            return "1";
        }
    }),

    SYSLOG_RECONNECT_INTERVAL(new ConfigSetter<String>("syslog.reconnect_interval") {
        @Override
        public synchronized void setValue(String syslogReconnectInterval) {
            int value = Integer.parseInt(syslogReconnectInterval);
            if (value <= 0) {
                throw new ConfigLoadException(itemName + " must be greater than 0");
            }
            Config.getConfig().syslogReconnectInterval = value;
        }

        @Override
        public String getDefaultValue() {
            return "300000";
        }
    }),

    LOG_MAXBURST(new ConfigSetter<String>("log.maxburst") {
        @Override
        public synchronized void setValue(String logMaxBurst) {
            int value = Integer.parseInt(logMaxBurst);
            if (value < 0) {
                throw new ConfigLoadException(itemName + " can not be less than 0");
            }
            Config.getConfig().logMaxBurst = value;
        }

        @Override
        public String getDefaultValue() {
            return "100";
        }
    }),

    HEARTBEAT_INTERVAL(new ConfigSetter<String>("cloud.heartbeat_interval") {
        @Override
        public synchronized void setValue(String heartbeatInterval) {
            int value = Integer.parseInt(heartbeatInterval);
            if (!(value >= 10 && value <= 1800)) {
                throw new ConfigLoadException(itemName + " must be between [10,1800]");
            }
            Config.getConfig().heartbeatInterval = value;
        }

        @Override
        public String getDefaultValue() {
            return "90";
        }
    }),

    HOOK_WHITE(new ConfigSetter<Map<String, Object>>("hook.white") {
        @Override
        public synchronized void setValue(Map<String, Object> heartbeatInterval) {
            TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
            temp.putAll(HookWhiteModel.parseHookWhite(heartbeatInterval));
            HookWhiteModel.init(temp);
        }

        @Override
        public Map<String, Object> getDefaultValue() {
            return new HashMap<String, Object>();
        }
    }),

    HOOK_WHITE_ALL(new ConfigSetter<String>("hook.white.ALL") {
        @Override
        public synchronized void setValue(String hookWhiteAll) {
            Config.getConfig().hookWhiteAll = Boolean.parseBoolean(hookWhiteAll);
        }

        @Override
        public String getDefaultValue() {
            return "true";
        }
    }),

    DECOMPILE_ENABLE(new ConfigSetter<String>("decompile.enable") {
        @Override
        public synchronized void setValue(String decompileEnable) {
            Config.getConfig().decompileEnable = Boolean.parseBoolean(decompileEnable);
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    RESPONSE_HEADERS(new ConfigSetter<HashMap<String, String>>("inject.custom_headers") {
        @Override
        public synchronized void setValue(HashMap<String, String> responseHeaders) {
            Config.getConfig().responseHeaders = responseHeaders;
        }

        @Override
        public HashMap<String, String> getDefaultValue() {
            HashMap<String, String> headers = new HashMap<String, String>();
            headers.put(HookHandler.OPEN_RASP_HEADER_KEY, HookHandler.OPEN_RASP_HEADER_VALUE);
            return headers;
        }
    }),

    DEPENDENCY_CHECK_INTERVAL(new ConfigSetter<String>("dependency_check.interval") {
        @Override
        public synchronized void setValue(String dependencyCheckInterval) {
            int value = Integer.parseInt(dependencyCheckInterval);
            if (value < 60 || value > 24 * 3600) {
                throw new ConfigLoadException(itemName + " must be between [60,86400]");
            }
            Config.getConfig().dependencyCheckInterval = value;
        }

        @Override
        public String getDefaultValue() {
            return "21600";
        }
    }),

    SECURITY_WEAK_PASSWORDS(new ConfigSetter<List<String>>("security.weak_passwords") {
        @Override
        public synchronized void setValue(List<String> securityWeakPasswords) {
            if (securityWeakPasswords != null) {
                if (securityWeakPasswords.size() > 200) {
                    throw new ConfigLoadException("the length of " + itemName + " can not be greater than 200");
                } else {
                    for (String weak : securityWeakPasswords) {
                        if (weak.length() > 16) {
                            throw new ConfigLoadException("the length of each weak word can not be greater than 16");
                        }
                    }
                }
                Config.getConfig().securityWeakPasswords = securityWeakPasswords;
            }
        }

        @Override
        public List<String> getDefaultValue() {
            return null;
        }
    }),

    CPU_USAGE_PERCENT(new ConfigSetter<String>("cpu.usage.percent") {
        @Override
        public synchronized void setValue(String cpuUsagePercent) {
            int value = Integer.parseInt(cpuUsagePercent);
            if (!(value >= 30 && value <= 100)) {
                throw new ConfigLoadException(itemName + " must be between [30,100]");
            }
            Config.getConfig().cpuUsagePercent = value;
        }

        @Override
        public String getDefaultValue() {
            return "90";
        }
    }),

    CPU_USAGE_ENABLE(new ConfigSetter<String>("cpu.usage.enable") {
        @Override
        public synchronized void setValue(String cpuUsageEnable) {
            Config.getConfig().cpuUsageEnable = Boolean.parseBoolean(cpuUsageEnable);
            try {
                CpuMonitorManager.resume(Config.getConfig().cpuUsageEnable);
            } catch (Throwable t) {
                // ignore 避免发生异常造成死循环
            }
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    CPU_USAGE_INTERVAL(new ConfigSetter<String>("cpu.usage.interval") {
        @Override
        public synchronized void setValue(String cpuUsageCheckInterval) {
            int interval = Integer.parseInt(cpuUsageCheckInterval);
            if (interval > 1800 || interval < 1) {
                throw new ConfigLoadException(itemName + " must be between [1,1800]");
            }
            Config.getConfig().cpuUsageCheckInterval = interval;
        }

        @Override
        public String getDefaultValue() {
            return "5";
        }
    }),

    HTTPS_VERIFY_SSL(new ConfigSetter<String>("openrasp.ssl_verifypeer") {
        @Override
        public synchronized void setValue(String httpsVerifyPeer) {
            Config.getConfig().isHttpsVerifyPeer = Boolean.parseBoolean(httpsVerifyPeer);
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    });

    ConfigItem(ConfigSetter setter) {
        this(setter, true);
    }

    ConfigItem(ConfigSetter setter, boolean isProperties) {
        this.isProperties = isProperties;
        this.setter = setter;
    }

    boolean isProperties;
    ConfigSetter setter;

    @Override
    public String toString() {
        return setter.itemName;
    }
}