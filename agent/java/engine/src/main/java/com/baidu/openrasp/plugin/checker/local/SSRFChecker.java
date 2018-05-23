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

package com.baidu.openrasp.plugin.checker.local;


import com.baidu.openrasp.HookHandler;
import com.baidu.openrasp.config.Config;
import com.baidu.openrasp.plugin.checker.CheckParameter;
import com.baidu.openrasp.plugin.checker.js.JsChecker;
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.plugin.js.engine.JSContext;
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
    private static final String[] INTRANET_DETECTION_SUFFIX = new String[]{".xip.io", ".burpcollaborator.net",
            ".xip.name", ".nip.io", ".vcap.me"};

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        List<EventInfo> result = new LinkedList<EventInfo>();
        String hostName = (String) checkParameter.getParam("hostname");
        String url = (String) checkParameter.getParam("url");
        NativeArray ips = (NativeArray) checkParameter.getParam("ip");
        try {
            JsonObject config = Config.getConfig().getAlgorithmConfig();
            Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
            if (!isModuleIgnore(config, CONFIG_KEY_SSRF_USER_INPUT)) {
                if (ips.size() > 0) {
                    for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                        String[] v = entry.getValue();
                        String value = v[0];
                        String ip = (String) ips.get(0);
                        if (url.equals(value) && Pattern.matches("^(192|172|10)\\..*", ip)) {
                            result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                                    getActionElement(config, CONFIG_KEY_SSRF_USER_INPUT), "访问内网地址: " + ip));
                        }
                    }
                }
            }

            if (result.isEmpty() && !isModuleIgnore(config, CONFIG_KEY_SSRF_COMMON)) {
                boolean isFound = false;
                for (String suffix : INTRANET_DETECTION_SUFFIX) {
                    if (hostName.endsWith(suffix)) {
                        isFound = true;
                        break;
                    }
                }
                if (isFound || hostName.equals("requestb.in")) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                            getActionElement(config, CONFIG_KEY_SSRF_COMMON), "访问已知的内网探测域名"));
                }
            }

            if (result.isEmpty()) {
                if (!isModuleIgnore(config, CONFIG_KEY_SSRF_AWS)
                        && hostName.equals("169.254.169.254")) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                            getActionElement(config, CONFIG_KEY_SSRF_AWS), "尝试读取 AWS metadata"));
                } else if (!isModuleIgnore(config, CONFIG_KEY_SSRF_OBFUSCATE)
                        && StringUtils.isNumeric(hostName)) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                            getActionElement(config, CONFIG_KEY_SSRF_OBFUSCATE), "尝试使用纯数字IP"));
                } else if (!isModuleIgnore(config, CONFIG_KEY_SSRF_OBFUSCATE)
                        && hostName.startsWith("0x") && !hostName.contains(".")) {
                    result.add(AttackInfo.createLocalAttackInfo(checkParameter,
                            getActionElement(config, CONFIG_KEY_SSRF_OBFUSCATE), "尝试使用16进制IP"));
                }
            }
        } catch (Exception e) {
            JSContext.LOGGER.warn("An error occurred while the local ssrf plugin was detecting, because:" + e.getMessage());
        }

        List<EventInfo> jsResults = new JsChecker().checkParam(checkParameter);
        if (jsResults != null && jsResults.size() > 0) {
            result.addAll(jsResults);
        }
        return result;
    }

    private boolean isModuleIgnore(JsonObject config, String configKey) {
        String action = getActionElement(config, configKey);
        return EventInfo.CHECK_ACTION_IGNORE.equals(action) || action == null;
    }

}
