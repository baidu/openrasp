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

package com.baidu.openrasp.plugin.checker.local;


import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.cloud.model.ErrorType;
import com.baidu.openrasp.cloud.utils.CloudUtils;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.engine.JSContext;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.apache.commons.lang3.StringUtils;
import org.mozilla.javascript.NativeArray;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * Created by tyy on 17-12-20.mes
 *
 * SSRF 检测 java 版本
 */
public class SSRFChecker extends ConfigurableChecker {

    private static final String CONFIG_KEY_SSRF_AWS = "ssrf_aws";
    private static final String CONFIG_KEY_SSRF_COMMON = "ssrf_common";
    private static final String CONFIG_KEY_SSRF_OBFUSCATE = "ssrf_obfuscate";
    private static final String CONFIG_KEY_SSRF_USER_INPUT = "ssrf_userinput";

    public List<EventInfo> checkSSRF(CheckParameter checkParameter, Map<String, String[]> parameterMap, JsonObject config) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        String hostName = (String) checkParameter.getParam("hostname");
        String url = (String) checkParameter.getParam("url");
        List ips = (List) checkParameter.getParam("ip");

        if (!isModuleIgnore(config, CONFIG_KEY_SSRF_USER_INPUT)) {             
            for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                String[] v = entry.getValue();
                String value = v[0];

                if (url.equals(value)) {

                    // 拦截内网地址
                    if (ips.size() > 0) 
                    {
                        String ip = (String) ips.get(0);
                        if (Pattern.matches("^(127|192|172|10)\\..*", ip)) {
                            result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                                getActionElement(config, CONFIG_KEY_SSRF_USER_INPUT),
                                "SSRF - Requesting intranet address: " + ip, "ssrf_userinput"));
                        }
                    } 
                    
                    // 拦截 localhost 另类写法
                    if ("[::]".equals(hostName)) 
                    {
                        result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                            getActionElement(config, CONFIG_KEY_SSRF_USER_INPUT),
                            "SSRF - Requesting intranet address: " + hostName, "ssrf_userinput"));
                    }
                }
            }            
        }

        if (result.isEmpty() && !isModuleIgnore(config, CONFIG_KEY_SSRF_COMMON)) {
            boolean isFound = false;
            JsonArray domains = getJsonObjectAsArray(config, CONFIG_KEY_SSRF_COMMON, "domains");
            if (domains != null) {
                for (JsonElement suffix : domains) {
                    if (hostName.endsWith(suffix.getAsString())) {
                        isFound = true;
                        break;
                    }
                }
            }
            if (isFound || hostName.equals("requestb.in")) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, getActionElement(config,
                        CONFIG_KEY_SSRF_COMMON), "SSRF - Requesting known DNSLOG address: " + hostName, "ssrf_common"));
            }
        }

        if (result.isEmpty()) {
            if (!isModuleIgnore(config, CONFIG_KEY_SSRF_AWS)
                    && hostName.equals("169.254.169.254")) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, getActionElement(config,
                        CONFIG_KEY_SSRF_AWS), "SSRF - Requesting AWS metadata address", "ssrf_aws"));
            } else if (!isModuleIgnore(config, CONFIG_KEY_SSRF_OBFUSCATE)
                    && StringUtils.isNumeric(hostName)) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, getActionElement(config,
                        CONFIG_KEY_SSRF_OBFUSCATE), "SSRF - Requesting numeric IP address", "ssrf_obfuscate"));
            } else if (!isModuleIgnore(config, CONFIG_KEY_SSRF_OBFUSCATE)
                    && hostName.startsWith("0x") && !hostName.contains(".")) {
                result.add(AttackInfo.createLocalAttackInfo(checkParameter, getActionElement(config,
                        CONFIG_KEY_SSRF_OBFUSCATE), "SSRF - Requesting hexadecimal IP address", "ssrf_obfuscate"));
            }
        }
        return result;
    }

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
        try {
            result = checkSSRF(checkParameter, parameterMap, config);
        } catch (Exception e) {
            String message = "Exception while executing builtin SSRF plugin, was:" + e.getMessage();
            int errorCode = ErrorType.PLUGIN_ERROR.getCode();
            JSContext.LOGGER.warn(CloudUtils.getExceptionObject(message, errorCode), e);
        }

        List<EventInfo> jsResults = new JsChecker().checkParam(checkParameter);
        if (jsResults != null && jsResults.size() > 0) {
            result.addAll(jsResults);
        }
        // 检测无威胁的url加入lru缓存
        if (result.isEmpty()) {
            if (HookHandler.commonLRUCache.maxSize() != 0) {
                HookHandler.commonLRUCache.put(new Gson().toJson(checkParameter.getParams()), null);
            }
        }
        return result;
    }

    private boolean isModuleIgnore(JsonObject config, String configKey) {
        String action = getActionElement(config, configKey);
        return EventInfo.CHECK_ACTION_IGNORE.equals(action) || action == null;
    }

}
