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

import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.HookWhiteModel;
import com.baidu.openrasp.detector.ServerDetector;
import com.baidu.openrasp.exceptions.ConfigLoadException;
import com.baidu.openrasp.tool.LRUCache;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.cpumonitor.CpuMonitorManager;
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
                AlgorithmConfigUtil.setSqlErrorCodes();
            } catch (Exception e) {
                Config.LOGGER.warn(
                        "failed to get the error_code element from algorithm config: " + e.getMessage(), e);
            }

            try {
                AlgorithmConfigUtil.setLogRegexes();
            } catch (Exception e) {
                Config.LOGGER.warn(
                        "failed to get the log_regex element from algorithm config: " + e.getMessage(), e);
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

    HOOK_WHITE(new ConfigSetter<Map<Object, Object>>("hook.white") {
        @Override
        public synchronized void setValue(Map<Object, Object> hookWhite) {
            TreeMap<String, Integer> temp = new TreeMap<String, Integer>();
            temp.putAll(HookWhiteModel.parseHookWhite(hookWhite));
            HookWhiteModel.init(temp);
        }

        @Override
        public Map<Object, Object> getDefaultValue() {
            return new HashMap<Object, Object>();
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

    RESPONSE_HEADERS(new ConfigSetter<Map<Object, Object>>("inject.custom_headers") {
        @Override
        public synchronized void setValue(Map<Object, Object> responseHeaders) {
            for (Map.Entry<Object, Object> entry : responseHeaders.entrySet()) {
                Object k = entry.getKey();
                Object v = entry.getValue();
                if (k == null || v == null) {
                    throw new ConfigLoadException("the value of " + itemName +
                            "'s key and value can not be null");
                }
                if (!Reflection.isPrimitiveType(v) && !(v instanceof String)) {
                    throw new ConfigLoadException("the type of " + itemName +
                            "'s value must be primitive type or String, can not be " + v.getClass().getName());
                }
                String key = v.toString();
                String value = v.toString();
                if (key.length() == 0 || key.length() > 200) {
                    throw new ConfigLoadException("the length of " + itemName +
                            "'s key must be between [1,200]");
                }
                if (value.length() == 0 || value.length() > 200) {
                    throw new ConfigLoadException("the length of " + itemName +
                            "'s value must be between [1,200]");
                }
            }
            Config.getConfig().responseHeaders = responseHeaders;
        }

        @Override
        public Map<Object, Object> getDefaultValue() {
            HashMap<Object, Object> headers = new HashMap<Object, Object>();
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
                ServerDetector.checkServerPolicy();
            }
        }

        @Override
        public List<String> getDefaultValue() {
            return Arrays.asList("111111", "123", "123123", "123456", "123456a",
                    "a123456", "admin", "both", "manager", "mysql",
                    "root", "rootweblogic", "tomcat", "user",
                    "weblogic1", "weblogic123", "welcome1");
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
    }),

    IAST_ENABLE(new ConfigSetter<String>("iast.enable") {
        @Override
        public synchronized void setValue(String iastEnable) {
            Config.getConfig().iastEnable = Boolean.parseBoolean(iastEnable);
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    LRU_COMPARE_ENABLE(new ConfigSetter<String>("lru.compare_enable") {
        @Override
        public synchronized void setValue(String lruCompareEnable) {
            boolean value = Boolean.parseBoolean(lruCompareEnable);
            if (value != Config.getConfig().lruCompareEnable) {
                Config.getConfig().lruCompareEnable = value;
                Config.commonLRUCache.clear();
            }
        }

        @Override
        public String getDefaultValue() {
            return "false";
        }
    }),

    LRU_COMPARE_LIMIT(new ConfigSetter<String>("lru.compare_limit") {
        @Override
        public synchronized void setValue(String lruCompareLimit) {
            int value = Integer.parseInt(lruCompareLimit);
            if (value <= 0 || value > 102400) {
                throw new ConfigLoadException(itemName + " must be between [1,102400]");
            }
            if (value < Config.getConfig().lruCompareLimit) {
                Config.commonLRUCache.clear();
            }
            Config.getConfig().lruCompareLimit = value;
        }

        @Override
        public String getDefaultValue() {
            return "10240";
        }
    }),

    RESPONSE_SAMPLER_INTERVAL(new ConfigSetter<String>("response.sampler_interval") {
        @Override
        public synchronized void setValue(String interval) {
            int responseSamplerInterval = Integer.parseInt(interval);
            if (responseSamplerInterval < 60) {
                throw new ConfigLoadException(itemName + " must be between [60,+∞)");
            }
            Config.getConfig().responseSamplerInterval = responseSamplerInterval;
        }

        @Override
        public String getDefaultValue() {
            return "60";
        }
    }),

    RESPONSE_SAMPLER_BURST(new ConfigSetter<String>("response.sampler_burst") {
        @Override
        public synchronized void setValue(String burst) {
            int responseSamplerBurst = Integer.parseInt(burst);
            if (responseSamplerBurst < 0) {
                throw new ConfigLoadException(itemName + " must not be negative");
            }
            Config.getConfig().responseSamplerBurst = responseSamplerBurst;
        }

        @Override
        public String getDefaultValue() {
            return "5";
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