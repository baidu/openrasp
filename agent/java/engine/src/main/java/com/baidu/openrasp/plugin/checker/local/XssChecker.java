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
import com.baidu.openrasp.plugin.info.AttackInfo;
import com.baidu.openrasp.plugin.info.EventInfo;
import com.baidu.openrasp.tool.Reflection;
import com.baidu.openrasp.tool.model.ApplicationModel;
import com.google.gson.JsonObject;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class XssChecker extends ConfigurableChecker {
    private static final String CONFIG_KEY_XSS_USER_INPUT = "xss_userinput";
    private static final String EXCEED_LENGTH_COUNT = "max_detection_num";
    private static final String XSS_PARAMETER_LENGTH = "min_length";
    private static final String XSS_REGEX = "filter_regex";
    private static final int DEFAULT_MIN_LENGTH = 15;
    private static final int DEFAULT_MAX_DETECTION_NUM = 10;
    private static final String DEFAULT_XSS_REGEX = "<![\\-\\[A-Za-z]|<([A-Za-z]{1,12})[\\/ >]";

    @Override
    public List<EventInfo> checkParam(CheckParameter checkParameter) {
        JsonObject config = Config.getConfig().getAlgorithmConfig();
        String action = getActionElement(config, CONFIG_KEY_XSS_USER_INPUT);
        LinkedList<EventInfo> result = new LinkedList<EventInfo>();
        String content = String.valueOf(checkParameter.getParam("html_body"));
        if (!EventInfo.CHECK_ACTION_IGNORE.equals(action)) {
            if (HookHandler.requestCache.get() != null && content != null) {
                Map<String, String[]> parameterMap = HookHandler.requestCache.get().getParameterMap();
                if (parameterMap != null) {
                    String regex = getStringElement(config, CONFIG_KEY_XSS_USER_INPUT, XSS_REGEX);
                    if (regex == null) {
                        regex = DEFAULT_XSS_REGEX;
                    }
                    Pattern pattern = Pattern.compile(regex);

                    int xssParameterLength = getIntElement(config, CONFIG_KEY_XSS_USER_INPUT, XSS_PARAMETER_LENGTH);
                    if (xssParameterLength < 0) {
                        xssParameterLength = DEFAULT_MIN_LENGTH;
                    }
                    int exceedLengthCount = getIntElement(config, CONFIG_KEY_XSS_USER_INPUT, EXCEED_LENGTH_COUNT);
                    if (exceedLengthCount < 0) {
                        exceedLengthCount = DEFAULT_MAX_DETECTION_NUM;
                    }
                    int count = 0;
                    for (Map.Entry<String, String[]> entry : parameterMap.entrySet()) {
                        for (String value : entry.getValue()) {
                            Matcher matcher = pattern.matcher(value);
                            boolean isMatch = matcher.find();
                            if (value.length() >= xssParameterLength && isMatch) {
                                count++;
                                if (content.contains(value)) {
                                    if ("websphere".equals(ApplicationModel.getServerName())) {
                                        Reflection.invokeMethod(HookHandler.responseCache.get(), "resetBuffer", new Class[]{});
                                    }
                                    String message = "Reflected XSS attack detected, parameter name: " + entry.getKey();
                                    Map<String, Object> params = (Map<String, Object>) checkParameter.getParams();
                                    params.remove("html_body");
                                    params.put("name", entry.getKey());
                                    params.put("value", value);
                                    result.add(AttackInfo.createLocalAttackInfo(checkParameter, action, message, CONFIG_KEY_XSS_USER_INPUT));
                                    return result;
                                }
                                if (count > exceedLengthCount) {
                                    return result;
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }
}
